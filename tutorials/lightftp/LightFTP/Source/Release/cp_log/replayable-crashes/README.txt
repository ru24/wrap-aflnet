Command line used to find this crash:

/home/23mUeno/wrap/aflnet/afl-fuzz -d -i /home/23mUeno/wrap/aflnet/tutorials/lightftp/in-ftp -o out-lightftp101 -N tcp://127.0.0.1/2200 -x /home/23mUeno/wrap/aflnet/tutorials/lightftp/ftp.dict -t 100 -P FTP -D 10000 -q 3 -s 3 -E -R -c ./ftpclean.sh ./fftp fftp.conf 2200

If you can't reproduce a bug outside of afl-fuzz, be sure to set the same
memory limit. The limit used for this fuzzing session was 50.0 MB.

Need a tool to minimize test cases before investigating the crashes or sending
them to a vendor? Check out the afl-tmin that comes with the fuzzer!

Found any cool bugs in open-source tools using afl-fuzz? If yes, please drop
me a mail at <lcamtuf@coredump.cx> once the issues are fixed - I'd love to
add your finds to the gallery at:

  http://lcamtuf.coredump.cx/afl/

Thanks :-)
