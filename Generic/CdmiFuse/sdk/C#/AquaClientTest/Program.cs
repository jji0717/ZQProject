using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using XOR_Media.AquaClient;

namespace AquaClientNet
{
    public class AquaClientTest
    {
        static void copy(String srcFile, String destFile)
        {
            bool bFromAqua = (0 == srcFile.IndexOf("FUSE:"));
            bool bToAqua = (0 == destFile.IndexOf("FUSE:"));

            String rootURL = "http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi";
            String homeContainer = "/default/npvr";
            String jsonProps = "";
            AquaClientRef client = new AquaClientRef(rootURL, homeContainer, jsonProps);

            try
            {
                FileStream fcIn = null, fcOut = null;

                if (!bFromAqua)
                {
                    Console.WriteLine("opening external input file: " + srcFile);
                    fcIn = new FileStream(srcFile, FileMode.Open);
                }
                else
                    srcFile = srcFile.Substring(5); // cut off the leading "FUSE:"

                if (!bToAqua)
                {
                    Console.WriteLine("opening external output file: " + destFile);
                    fcOut = new FileStream(srcFile, FileMode.Create);
                }
                else
                    destFile = destFile.Substring(5); // cut off the leading "FUSE:"

                int buffSize = 100 * 1024;
                byte[] dataBuf = new byte[buffSize];

                int len = 0;
                int offset = 0;

                StringBuilder location = new StringBuilder();
                StringBuilder contentType = new StringBuilder();

                do
                {
                    Array.Clear(dataBuf, 0, dataBuf.Length);

                    if (bFromAqua)
                    {
                        len = dataBuf.Length;
                        LongVariable nread = new LongVariable(0);
                        int ret = client.nonCdmi_ReadDataObject(srcFile, contentType,
                                location, offset, nread, dataBuf, len, false);
                        if (ret >= 200 && ret < 300)
                            len = (int)nread.get();

                        Console.WriteLine("read aqua ret=" + ret + " len=" + len);

                    }
                    else
                        // len = fis.read(dataBuf);
                        len = fcIn.Read(dataBuf, 0, buffSize);

                    if (len <= 0)
                        break;

                    Console.WriteLine("copying " + offset + "+" + len);
                    if (bToAqua)
                    {
                        int ret = client.nonCdmi_UpdateDataObject(destFile,
                                location, "", offset, -1, dataBuf, len, true, false);
                        if (ret < 200 || ret >= 300)
                            break;
                    }
                    else
                        // fos.write(dataBuf, 0, len);
                        fcOut.Write(dataBuf, 0, len);

                    offset += len;
                } while (len > 0);

                Console.WriteLine("file copied, size=" + offset);

                if (!bFromAqua)
                    // fis.close();
                    fcIn.Close();

                if (!bToAqua)
                    // fos.close();
                    fcOut.Close();
                else
                    client.flushDataObject(destFile);
            }
            catch (FileNotFoundException ex)
            {
                Console.WriteLine(ex.Message + " in the specified directory.");
            }
            catch (IOException e)
            {
                Console.WriteLine(e.Message);
            }
        }

        static void get(String srcFileOnAqua, String destFileOnLocal)
        {

            // step 1. instantiate a client to connect to Aqua

            String rootURL = "http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi";
            String homeContainer = "/hongquan/test999";
            String jsonProps = "";
            AquaClientRef client = new AquaClientRef(rootURL, homeContainer, jsonProps);

            try
            {

                // step 2. open the output file on local disk to save
                FileStream fcOut = null;

                Console.WriteLine("opening external output file: " + destFileOnLocal);
                fcOut = new FileStream(destFileOnLocal, FileMode.Create);

                // step 3. allocate a Byte array dataBuf. be aware this buffer must be allocated via direct mode
                int buffSize = 100 * 1024;
                byte[] dataBuf = new byte[buffSize];

                // step 4. copy the data buffer by buffer and save to the local file
                int len = 0;
                int offset = 0;

                // the output string parameters from API nonCdmi_ReadDataObject()
                StringBuilder location = new StringBuilder();
                StringBuilder contentType = new StringBuilder();

                // the loop
                do
                {
                    // step 4.1 clean up the data left in the dataBuf
                    Array.Clear(dataBuf, 0, dataBuf.Length);

                    // step 4.2 initialize the output nread with the capacity of dataBuf
                    len = dataBuf.Length;

                    LongVariable nread = new LongVariable(0);

                    // step 4.3 invokes nonCdmi_ReadDataObject() to read data range [offset, offset+dataBuf.capacity()] into dataBuf 
                    int ret = client.nonCdmi_ReadDataObject(srcFileOnAqua, contentType,
                                location, offset, nread, dataBuf, len, false);

                    Console.WriteLine("read aqua contenttype=" + contentType + " location=" + location);

                    // step 4.4 check the return code
                    if (ret >= 200 && ret < 300)
                        len = (int)nread.get();
                    else len = 0;

                    Console.WriteLine("read aqua ret=" + ret + " len=" + len);

                    // step 4.5 quit the loop if error occurs or reached the EOF
                    if (len <= 0)
                        break;

                    // step 4.6 write the received dataBuf into the local file
                    Console.WriteLine("copying " + offset + "+" + len);
                    fcOut.Write(dataBuf, 0, len);

                   //setp4.7 file eof
 //                   if(len < dataBuf.Length)
 //                      break;

                    // step 4.7 step the offset for next round
                    offset += len;
                } while (len > 0);

                // step 5. finish getting, close the local file
                Console.WriteLine("file copied, size=" + offset);
                fcOut.Close();

            }
            catch (FileNotFoundException ex)
            {
                Console.WriteLine(ex.Message + " in the specified directory.");
            }
            catch (IOException e)
            {
                Console.WriteLine(e.Message);
            }
        }

        public static void put(String srcFileOnLocal, String destFileOnAqua) 
      {
               // step 1. instantiate a client to connect to Aqua
            String rootURL ="http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi";
            String homeContainer = "/hongquan/test999";
            String jsonProps="";
            AquaClientRef client = new AquaClientRef(rootURL, homeContainer, jsonProps);

               try {

                   // step 2. open the output file on local disk to save
                   FileStream fcIn = null;
                   Console.WriteLine("opening external input file: " + srcFileOnLocal);

                   fcIn = new FileStream(srcFileOnLocal, FileMode.Open);
			
                   // step 3. allocate a ByteBuffer dataBuf. be aware this buffer must be allocated via direct mode
                   int buffSize = 100 * 1024;
                   byte[] dataBuf = new byte[buffSize];

                   // step 4 create the new data object on Aqua
                   Console.WriteLine("creating " + destFileOnAqua + " on Aqua");

                   client.nonCdmi_CreateDataObject(destFileOnAqua, "", dataBuf, 0);

                   // step 5. copy the data buffer by buffer and save to the local file
                   int len = 0;
                   int offset = 0;

                   // the output string parameters from API nonCdmi_ReadDataObject()
                  StringBuilder location = new StringBuilder();
                  StringBuilder contentType = new StringBuilder();

                   // the loop
                   do {
				
                       // step 5.1 clean up the data left in the dataBuf
                      Array.Clear(dataBuf, 0, dataBuf.Length);
                      // step 5.2 read the data from the local file
                      len = fcIn.Read(dataBuf, 0, dataBuf.Length);

                       // step 5.3 check the len of data read, see if we have reached EOF
                       if (len <= 0)
                           break;

                       // step 5.4 invokes nonCdmi_UpdateDataObject() to write the buffer into the data object on Aqua 
                       Console.WriteLine("copying " + offset + "+" + len);
                       int ret = client.nonCdmi_UpdateDataObject(destFileOnAqua,
                                   location, "", offset, -1, dataBuf, len, true, false);

                       // step 5.6 check the return code
                       if (ret < 200 || ret >= 300)
                               break;

                       // step 5.7 step the offset for next round
                       offset += len;
                   } while (len > 0);

                   // step 6. finish putting, close the local file
                   Console.WriteLine("file copied, size=" + offset);
                   fcIn.Close();
			
                   // step 7. flush the data in cache onto Aqua if there are any
                   client.flushDataObject(destFileOnAqua);
			
               } 
               catch (FileNotFoundException ex) 
               {
                   Console.WriteLine(ex.Message + " in the specified directory.");
               } 
               catch (IOException e)
               {
                  Console.WriteLine(e.Message);
               }
           }

           static void Main(string[] args)
           {
               // initialize the SDK by setting the log.level=DEBUG
               Dictionary<String, String> config = new Dictionary<String, String>();
               config.Add("log.level", "7");

               if (!AquaClientRef.initSDK(config))
               {
                   Console.WriteLine("failed to init aquaclient sdk");
                   return;
               }

               // dispatch the sub-commands
               do
               {
                   if (args.Length <= 1)
                       break; // no sub command specified

                   if (args[0].Equals("get"))
                   {
                       if (args.Length < 3)
                       {
                           Console.WriteLine("get: please specify the source and destination file names");
                           break;
                       }

                       Console.WriteLine("copy file " + args[1] + " to " + args[2]);
                       get(args[1], args[2]);
                       break;
                   }

                   if (args[0].Equals("put"))
                   {
                       if (args.Length < 3)
                       {
                           Console.WriteLine("put: please specify the source and destination file names");
                           break;
                       }

                       Console.WriteLine("copy file " + args[1] + " to " + args[2]);
                       put(args[1], args[2]);
                       break;
                   }

               } while (false);

               // uninitialize the SDK
               Console.WriteLine("test finished");
               AquaClientRef.uninitSDK();
               Console.WriteLine("sdk uninitialized");
           }
       }
}
