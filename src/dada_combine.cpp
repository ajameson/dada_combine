

// psrdada includes
#include <ascii_header.h>

// standard includes
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cinttypes>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void usage()
{
  std::cout << "dada_combine [options] lower-input-file upper-input-file output-file" << std::endl;
  std::cout << "Combines 2 half-band baseband voltage PSRDADA files from PTUSE." << std::endl;
  std::cout << "  lower-input-file   input PSRDADA file from lower half-band" << std::endl;
  std::cout << "  upper-input-file   input PSRDADA file from upper half-band" << std::endl;
  std::cout << "  output-file        combined output PSRDADA file to create" << std::endl;
  std::cout << "  -h                 display this help text" << std::endl;
  std::cout << "  -v                 increase the verbosity of this application" << std::endl;
}

uint64_t combine_uint64(char * key, char * lower_hdr, char * upper_hdr, char * output_hdr)
{
  uint64_t lower_val{0}, upper_val{0};

  if (ascii_header_get(lower_hdr, key, "%ld", &lower_val) != 1)
  {
    std::cerr << "dada_combine: could not read " << key << " from lower_hdr" << std::endl;
    assert(false);
  }
  if (ascii_header_get(upper_hdr, key, "%ld", &upper_val) != 1)
  {
    std::cerr << "dada_combine: could not read " << key << " from upper_hdr" << std::endl;
    assert(false);
  }

  // require/expect that the uint64 values equal for lower and upper
  if (lower_val != upper_val)
  {
    std::cerr << "dada_combine: lower " << key << "=" << lower_val << " != upper " << key << "=" << upper_val << std::endl;
    assert(false);
  }

  uint64_t output_val = lower_val + upper_val;
  if (ascii_header_set(output_hdr, key, "%ld", output_val) < 0)
  {
    std::cerr << "dada_combine: could not write" << key << "=" << output_val << " to output_hdr" << std::endl;
    assert(false);
  }
  return output_val;
}

void get_double(char * key, char * lower_hdr, char * upper_hdr, double * lower_val, double * upper_val)
{
  if (ascii_header_get(lower_hdr, key, "%f", &lower_val) != 1)
  {
    std::cerr << "dada_combine: could not read " << key << " from lower_hdr" << std::endl;
    assert(false);
  }
  if (ascii_header_get(upper_hdr, key, "%f", &upper_val) != 1)
  {
    std::cerr << "dada_combine: could not read " << key << " from upper_hdr" << std::endl;
    assert(false);
  }
}

void set_double(char * key, char * output_hdr, double output_val)
{
  if (ascii_header_set(output_hdr, key, "%f", output_val) < 0)
  {
    std::cerr << "dada_combine: could not write" << key << "=" << output_val << " to output_hdr" << std::endl;
    assert(false);
  }
}

int main(int argc, char *argv[])
{
  int arg = 0;
  int verbose = 0;

  while ((arg=getopt(argc,argv,"hv")) != -1)
  {
    switch (arg)
    {
      case 'h':
        usage();
        return EXIT_SUCCESS;

      case 'v':
        verbose++;
        break;

      default:
        std::cerr << "ERROR: unexpected option: " << optopt << std::endl;
        usage();
        return EXIT_FAILURE;
    }
  }

  int num_args = argc - optind;
  if (num_args != 3)
  {
    std::cerr << "ERROR: 3 command line argument required" << std::endl;
    usage();
    return EXIT_FAILURE;
  }

  std::string lower = std::string(argv[optind + 0]);
  std::string upper = std::string(argv[optind + 1]);
  std::string output = std::string(argv[optind + 2]);

  struct stat statistics;
  if (stat(lower.c_str(), &statistics) < 0)
  {
    std::cerr << "ERROR: failed to determine filesize for " << lower << std::endl;
    return EXIT_FAILURE;
  }
  size_t lower_size = size_t(statistics.st_size);

  if (stat(upper.c_str(), &statistics) < 0)
  {
    std::cerr << "ERROR: failed to determine filesize for " << upper << std::endl;
    return EXIT_FAILURE;
  }
  size_t upper_size = size_t(statistics.st_size);

  if (lower_size != upper_size)
  {
    std::cerr << "ERROR: lower_size [" << lower_size << "] != upper_size [" << upper_size << "]" << std::endl;
    return EXIT_FAILURE;
  }

  size_t lower_hdr_size = ascii_header_get_size(const_cast<char *>(lower.c_str()));
  if (lower_hdr_size < 0)
  {
    std::cerr << "lower header size could not be determined" << std::endl;
    return EXIT_FAILURE;
  }
  size_t upper_hdr_size = ascii_header_get_size(const_cast<char *>(upper.c_str()));
  if (upper_hdr_size < 0)
  {
    std::cerr << "lower header size could not be determined" << std::endl;
    return EXIT_FAILURE;
  }
  if (lower_hdr_size != upper_hdr_size)
  {
    std::cerr << "lower header size [" << lower_hdr_size << "] != upper header size [" << upper_hdr_size << "]" << std::endl;
    return EXIT_FAILURE;
  }
  size_t output_hdr_size = lower_hdr_size;

  int ro_flags = O_RDONLY;
  int rw_flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  int lower_fd = open(lower.c_str(), ro_flags, perms);
  int upper_fd = open(upper.c_str(), ro_flags, perms);

  // read the ASCII header from the lower and upper file descriptors
  char * lower_hdr = reinterpret_cast<char *>(malloc(lower_hdr_size));
  char * upper_hdr = reinterpret_cast<char *>(malloc(upper_hdr_size));
  char * output_hdr = reinterpret_cast<char *>(malloc(output_hdr_size));

  // read header from lower and upper write to output
  size_t bytes_read = 0;
  bytes_read = ::read(lower_fd, lower_hdr, lower_hdr_size);
  assert(bytes_read == lower_hdr_size);
  bytes_read = ::read(upper_fd, upper_hdr, upper_hdr_size);
  assert(bytes_read == upper_hdr_size);

  // memcpy the lower header to the output, then modify
  memcpy(output_hdr, lower_hdr, lower_hdr_size);

  uint64_t output_resolution = combine_uint64("RESOLUTION", lower_hdr, upper_hdr, output_hdr);
  combine_uint64("OBS_OFFSET", lower_hdr, upper_hdr, output_hdr);
  combine_uint64("NCHAN", lower_hdr, upper_hdr, output_hdr);
  combine_uint64("BYTES_PER_SECOND", lower_hdr, upper_hdr, output_hdr);
  combine_uint64("FILE_SIZE", lower_hdr, upper_hdr, output_hdr);

  double lower_freq{0}, upper_freq{0};
  get_double("FREQ", lower_hdr, upper_hdr, &lower_freq, &upper_freq);
  double output_freq = (lower_freq + upper_freq) / 2;
  set_double("FREQ", output_hdr, output_freq);
  double lower_bw{0}, upper_bw{0};
  get_double("BW", lower_hdr, upper_hdr, &lower_bw, &upper_bw);
  double output_bw = lower_bw + upper_bw;
  set_double("BW", output_hdr, output_bw);

  if (upper_freq <= lower_freq)
  {
    std::cerr << "upper_freq [" << upper_freq << "] was <= lower_freq [" << lower_freq << "]" << std::endl;
    return EXIT_FAILURE; 
  }

  double diff_freq = upper_freq - lower_freq;
  if (abs(diff_freq - lower_bw) > 0.001)
  {
    std::cerr << "upper_freq-lower_freq [" << diff_freq << "] was != lower_bw [" << lower_bw << "]" << std::endl;
    return EXIT_FAILURE; 
  }

  double diff_bw = upper_bw - lower_bw;
  if (abs(diff_bw) > 0.001)
  {
    std::cerr << "upper_bw - lower_bw [" << diff_bw << "] was != 0" << std::endl;
    return EXIT_FAILURE; 
  }

  size_t data_size = lower_size - lower_hdr_size;
  size_t output_data_size = output_resolution * 2;
  if (output_data_size % output_resolution != 0)
  {
    std::cerr << "output_data_size [" << output_data_size << "] was not a multiple of output_resolution [" << output_resolution << "]" << std::endl;
    return EXIT_FAILURE;
  }

  // open the output file
  int output_fd = open(output.c_str(), rw_flags, perms);

  size_t bytes_written = ::write(output_fd, output_hdr, output_hdr_size);
  assert(bytes_written == output_hdr_size);

  // allocate buffers that are the size of the input resolution to read
  uint64_t input_resolution = output_resolution / 2;
  uint64_t pol_resolution = input_resolution / 2;
  void * buffer1 = malloc(pol_resolution);
  void * buffer2 = malloc(pol_resolution);

  // process blocks
  uint32_t nblocks = data_size / input_resolution;
  for (uint32_t iblock=0; iblock<nblocks; iblock++)
  {
    // read polA from lower and upper
    bytes_read = ::read(lower_fd, buffer1, pol_resolution);
    assert(bytes_read == pol_resolution);
    bytes_read = ::read(upper_fd, buffer2, pol_resolution);
    assert(bytes_read == pol_resolution);

    // write polA to output
    bytes_written = ::write(output_fd, buffer1, pol_resolution);
    assert(bytes_written == pol_resolution);
    bytes_written = ::write(output_fd, buffer2, pol_resolution);
    assert(bytes_written == pol_resolution);

    // read polB from lower and upper
    bytes_read = ::read(lower_fd, buffer1, pol_resolution);
    assert(bytes_read == pol_resolution);
    bytes_read = ::read(upper_fd, buffer2, pol_resolution);
    assert(bytes_read == pol_resolution);

    // write polB to output
    bytes_written = ::write(output_fd, buffer1, pol_resolution);
    assert(bytes_written == pol_resolution);
    bytes_written = ::write(output_fd, buffer2, pol_resolution);
    assert(bytes_written == pol_resolution);
  }

  close(lower_fd);
  close(upper_fd);
  close(output_fd);

  free(buffer1);
  free(buffer2);

  free(lower_hdr);
  free(upper_hdr);
  free(output_hdr);
}
