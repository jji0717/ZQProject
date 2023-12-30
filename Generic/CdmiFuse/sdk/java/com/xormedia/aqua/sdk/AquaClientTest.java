package com.xormedia.aqua.sdk;

import java.util.*;
import java.io.*;
import java.nio.*;
import java.nio.channels.FileChannel;

// http://json-lib.sourceforge.net/
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import net.sf.json.util.JSONUtils;

// $(JAVA_HOME)\bin\javac $(InputName).java; $(JAVA_HOME)\bin\javah $(InputName);

public class AquaClientTest {

	public static void copy(String srcFile, String destFile) {
		boolean bFromAqua = (0 == srcFile.indexOf("FUSE:"));
		boolean bToAqua = (0 == destFile.indexOf("FUSE:"));

		Properties props = new Properties();
		AquaClient client = AquaClient.newClient(
				"http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi",
				"", "/default/npvr", props);

		try {
			FileChannel fcIn = null, fcOut = null;

			if (!bFromAqua) {
				System.out.println("opening external input file: " + srcFile);
				fcIn = new RandomAccessFile(srcFile, "r").getChannel();
				// fis = new FileInputStream(srcFile);
			} else
				srcFile = srcFile.substring(5); // cut off the leading "FUSE:"

			if (!bToAqua) {
				System.out.println("opening external output file: " + destFile);
				fcOut = new RandomAccessFile(destFile, "rw").getChannel();
				// fos = new FileOutputStream(destFile);
			} else
				destFile = destFile.substring(5); // cut off the leading "FUSE:"

			ByteBuffer dataBuf = ByteBuffer.allocateDirect(100 * 1024);
			// byte[] buf = dataBuf.array();

			int len = 0;
			int offset = 0;
			StringBuffer location = new StringBuffer();
			StringBuffer contentType = new StringBuffer();
			do {
				dataBuf.clear();
				if (bFromAqua) {
					len = dataBuf.capacity();
					LongVariable nread = new LongVariable(len);
					int ret = client.nonCdmi_ReadDataObject(srcFile, contentType,
							location, offset, nread, dataBuf, false);
					if (ret >= 200 && ret < 300)
						len = (int) nread.get();
					System.out.println("read aqua ret=" + ret + " len=" + len);
				} else
					// len = fis.read(dataBuf);
					len = fcIn.read(dataBuf, offset);

				if (len <= 0)
					break;

				dataBuf.limit(len);

				System.out.println("copying " + offset + "+" + len);
				if (bToAqua) {
					int ret = client.nonCdmi_UpdateDataObject(destFile,
							location, "", offset, -1, dataBuf, true, false);
					if (ret < 200 || ret >= 300)
						break;
				} else
					// fos.write(dataBuf, 0, len);
					fcOut.write(dataBuf, offset);

				offset += len;
			} while (len > 0);

			System.out.println("file copied, size=" + offset);

			if (!bFromAqua)
				// fis.close();
				fcIn.close();

			if (!bToAqua)
				// fos.close();
				fcOut.close();
			else
				client.flushDataObject(destFile);
		} catch (FileNotFoundException ex) {
			System.out
					.println(ex.getMessage() + " in the specified directory.");
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}
	}

	public static void get(String srcFileOnAqua, String destFileOnLocal) {
		
		// step 1. instantiate a client to connect to Aqua
		Properties props = new Properties();
		AquaClient client = AquaClient.newClient(
				"http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi", "",
				"/default/npvr", props);

		try {
			
			// step 2. open the output file on local disk to save
			FileChannel fcOut = null;
			System.out.println("opening external output file: " + destFileOnLocal);
			fcOut = new RandomAccessFile(destFileOnLocal, "rw").getChannel();
			
			// step 3. allocate a ByteBuffer dataBuf. be aware this buffer must be allocated via direct mode
			ByteBuffer dataBuf = ByteBuffer.allocateDirect(100 * 1024);
			
			// step 4. copy the data buffer by buffer and save to the local file
			int len = 0;
			int offset = 0;
			
			// the output string parameters from API nonCdmi_ReadDataObject()
			StringBuffer location = new StringBuffer();
			StringBuffer contentType = new StringBuffer();
			
			// the loop
			do {
				// step 4.1 clean up the data left in the dataBuf
				dataBuf.clear();
				
				// step 4.2 initialize the output nread with the capacity of dataBuf
				len = dataBuf.capacity();
				LongVariable nread = new LongVariable(len);
				
				// step 4.3 invokes nonCdmi_ReadDataObject() to read data range [offset, offset+dataBuf.capacity()] into dataBuf 
				int ret = client.nonCdmi_ReadDataObject(srcFileOnAqua, contentType,
							location, offset, nread, dataBuf, false);
					
				// step 4.4 check the return code
				if (ret >= 200 && ret < 300)
					len = (int) nread.get();
				else len =0;
				
				System.out.println("read aqua ret=" + ret + " len=" + len);

				// step 4.5 quit the loop if error occurs or reached the EOF
				if (len <= 0)
					break;

				// step 4.6 limit the valid range within dataBuf
				dataBuf.limit(len);

				// step 4.7 write the received dataBuf into the local file
				System.out.println("copying " + offset + "+" + len);
				fcOut.write(dataBuf, offset);

				// step 4.8 step the offset for next round
				offset += len;
			} while (len > 0);

			// step 5. finish getting, close the local file
			System.out.println("file copied, size=" + offset);
			fcOut.close();
			
		} catch (FileNotFoundException ex) {
			System.out
					.println(ex.getMessage() + " in the specified directory.");
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}
	}
	
	public static void put(String srcFileOnLocal, String destFileOnAqua) {

		// step 1. instantiate a client to connect to Aqua
		Properties props = new Properties();
		AquaClient client = AquaClient.newClient(
				"http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi", "",
				"/default/npvr", props);

		try {

			// step 2. open the output file on local disk to save
			FileChannel fcIn = null;
			System.out.println("opening external input file: " + srcFileOnLocal);
			fcIn = new RandomAccessFile(srcFileOnLocal, "r").getChannel();
			
			// step 3. allocate a ByteBuffer dataBuf. be aware this buffer must be allocated via direct mode
			ByteBuffer dataBuf = ByteBuffer.allocateDirect(100 * 1024);
			dataBuf.clear(); 

			// step 4 create the new data object on Aqua
			System.out.println("creating " + destFileOnAqua + " on Aqua");
			dataBuf.limit(0);
			client.nonCdmi_CreateDataObject(destFileOnAqua, "", dataBuf);

			// step 5. copy the data buffer by buffer and save to the local file
			int len = 0;
			int offset = 0;

			// the output string parameters from API nonCdmi_ReadDataObject()
			StringBuffer location = new StringBuffer();
			StringBuffer contentType = new StringBuffer();

			// the loop
			do {
				
				// step 5.1 clean up the data left in the dataBuf
				dataBuf.clear();

				// step 5.2 read the data from the local file
				len = fcIn.read(dataBuf, offset);

				// step 5.3 check the len of data read, see if we have reached EOF
				if (len <= 0)
					break;

				// step 5.4 limit the valid range within dataBuf
				dataBuf.limit(len);

				// step 5.5 invokes nonCdmi_UpdateDataObject() to write the buffer into the data object on Aqua 
				System.out.println("copying " + offset + "+" + len);
				int ret = client.nonCdmi_UpdateDataObject(destFileOnAqua,
							location, "", offset, -1, dataBuf, true, false);

				// step 5.6 check the return code
				if (ret < 200 || ret >= 300)
						break;

				// step 5.7 step the offset for next round
				offset += len;
			} while (len > 0);

			// step 6. finish putting, close the local file
			System.out.println("file copied, size=" + offset);
			fcIn.close();
			
			// step 7. flush the data in cache onto Aqua if there are any
			client.flushDataObject(destFileOnAqua);
			
		} catch (FileNotFoundException ex) {
			System.out
					.println(ex.getMessage() + " in the specified directory.");
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}
	}

	public static void main(String[] args) {

		// initialize the SDK by setting the log.level=DEBUG
		Properties config = new Properties();
		config.put("log.level", 7);

		AquaClient.initSDK(config);

		// dispatch the sub-commands
		do {
			
			if (args.length <= 1)
				break; // no sub command specified
			
			if (args[0].equalsIgnoreCase("get")) {
				if (args.length < 3) {
					System.out.println("get: please specify the source and destination file names");
					break;
				}

				System.out.println("copy file " + args[1] + " to " + args[2]);
				get(args[1], args[2]);
				break;
			}
			
			if (args[0].equalsIgnoreCase("put")) {
				if (args.length < 3) {
					System.out.println("put: please specify the source and destination file names");
					break;
				}

				System.out.println("copy file " + args[1] + " to " + args[2]);
				put(args[1], args[2]);
				break;
			}

		} while (false);

		// uninitialize the SDK
		System.out.println("test finished");
		AquaClient.uninitSDK();
		System.out.println("sdk uninitialized");
	}

}
