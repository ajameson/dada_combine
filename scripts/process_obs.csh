#!/bin/tcsh

set beam_utc_source = $1

if ( "$1" == "" ) then
  echo "ERROR: 1 command line argument expected"
  exit
endif

set sb_count = `find /nvme/baseband/processing/$beam_utc_source -mindepth 1 -maxdepth 1 -type d -printf "%f\n" | wc -l`
if ( "$sb_count" != "2" ) then
  echo "ERROR: 2 sub-bands expected, found $sb_count"
  exit
endif
set sb_list = `find /nvme/baseband/processing/$beam_utc_source -mindepth 1 -maxdepth 1 -type d -printf "%f\n" | sort`
set wb = `echo $sb_list | awk '{printf "%d", int((($1 + $2)/2)+0.5)}'`

set file_count = `find  /nvme/baseband/processing/$beam_utc_source -mindepth 2 -maxdepth 2 -type f -name "????-??-??-??:??:??_*.000000.dada" -printf "%f\n" | sort -n | uniq | wc -l`
if ( "$file_count" == "0" ) then
  echo "ERROR: more than 1 dada file expected"
  exit
endif
set file_list = `find  /nvme/baseband/processing/$beam_utc_source -mindepth 2 -maxdepth 2 -type f -name "????-??-??-??:??:??_*.000000.dada" -printf "%f\n" | sort -n | uniq`

mkdir -p /store/baseband/send/$beam_utc_source/$wb

foreach file ( $file_list)

  set cmd = "dada_combine"
  foreach sb ( $sb_list )
    set cmd = "$cmd /nvme/baseband/processing/$beam_utc_source/$sb/$file"
  end
  set cmd = "$cmd /store/baseband/send/$beam_utc_source/$wb"
  echo $cmd
  $cmd

  if ( $? != 0 ) then
    echo "ERROR: $cmd failed"
    exit
  endif
end

set first_combined_file = `ls -1 /store/baseband/send/$beam_utc_source/$wb/????-??-??-??:??:??_0000000000000000.000000.dada`
if ( -f $first_combined_file ) then
  dada_edit $first_combined_file > /store/baseband/send/$beam_utc_source/$wb/obs.header
else
  echo "ERROR first combined file did not exist: $first_combined_file"
  exit
endif
