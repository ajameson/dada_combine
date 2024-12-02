
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cinttypes>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  int hdr_size = 4096;
  int npol = 2;
  int resolution = 524288;
  int pol_resolution = resolution / npol;

  int num_args = argc - optind;
  if (num_args != 3)
  {
    std::cerr << "ERROR: 3 command line argument required" << std::endl;
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

  size_t data_size = lower_size - hdr_size;
  if (data_size % resolution != 0)
  {
    std::cerr << "data_size [" << data_size << "] was not a multiple of resolution [" << resolution << "]" << std::endl;
    return EXIT_FAILURE;
  }

  int ro_flags = O_RDONLY;
  int rw_flags = O_WRONLY | O_CREAT | O_TRUNC;
  int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  int lower_fd = open(lower.c_str(), ro_flags, perms);
  int upper_fd = open(upper.c_str(), ro_flags, perms);
  int output_fd = open(output.c_str(), rw_flags, perms);

  void * buffer1 = malloc(pol_resolution);
  void * buffer2 = malloc(pol_resolution);

  // read header from lower, write to output
  size_t bytes_read = ::read(lower_fd, buffer1, hdr_size);
  assert(bytes_read == hdr_size);
  size_t bytes_written = ::write(output_fd, buffer1, hdr_size);
  assert(bytes_written == hdr_size);

  // also read header from upper
  bytes_read = ::read(upper_fd, buffer2, hdr_size);

  uint32_t nblocks = data_size / resolution;
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
}

