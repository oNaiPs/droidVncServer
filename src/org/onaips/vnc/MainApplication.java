package org.onaips.vnc;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Application;
import android.util.Log;

public class MainApplication extends Application {

	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();

		createBinary();
	}


	public void createBinary()  
	{ 
		copyBinary(R.raw.androidvncserver, getFilesDir().getAbsolutePath() + "/androidvncserver");
		copyBinary(R.raw.vncviewer, getFilesDir().getAbsolutePath()+"/VncViewer.jar");
		copyBinary(R.raw.indexvnc, getFilesDir().getAbsolutePath()+"/index.vnc");
		copyBinary(R.raw.busybox, getFilesDir().getAbsolutePath()+"/busybox");
		Process sh;
		try {
			sh = Runtime.getRuntime().exec("su");

			OutputStream os = sh.getOutputStream();

			//chmod 777 SHOULD exist
			writeCommand(os, "chmod 777 " + getFilesDir().getAbsolutePath() + "/androidvncserver");
			writeCommand(os, "chmod 777 " + getFilesDir().getAbsolutePath() +"/busybox");		
		} catch (IOException e) {
			Log.v("VNC",e.getMessage());		
		}catch (Exception e) {
			Log.v("VNC",e.getMessage());		
		}
	}



	public void copyBinary(int id,String path)
	{
		try {
			InputStream ins = getResources().openRawResource(id);
			int size = ins.available();

			// Read the entire resource into a local byte buffer.
			byte[] buffer = new byte[size];
			ins.read(buffer);
			ins.close();

			FileOutputStream fos = new FileOutputStream(path);
			fos.write(buffer);
			fos.close();
		}
		catch (Exception e)
		{
			Log.v("VNC","public void createBinary(): " + e.getMessage());
		}


	}  

	static void writeCommand(OutputStream os, String command) throws Exception
	{
		os.write((command + "\n").getBytes("ASCII"));
	} 
}
