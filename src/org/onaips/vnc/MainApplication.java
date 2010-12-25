package org.onaips.vnc;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Application;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.preference.PreferenceManager;
import android.util.Log;

public class MainApplication extends Application {

	@Override
	public void onCreate() {
		super.onCreate(); 

		if (firstRun())
			createBinary();
	}

	public boolean firstRun()
	{
		int versionCode = 0;
		try {
			versionCode = getPackageManager()
			.getPackageInfo(getPackageName(), PackageManager.GET_META_DATA)
			.versionCode;
		} catch (NameNotFoundException e) {
			Log.e("VNC", "Package not found... Odd, since we're in that package...", e);
		}

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		int lastFirstRun = prefs.getInt("last_run", 0);

		if (lastFirstRun >= versionCode) {
			Log.d("VNC", "Not first run");
			return false;
		}
		Log.d("VNC", "First run for version " + versionCode);

		SharedPreferences.Editor editor = prefs.edit();
		editor.putInt("last_run", versionCode);
		editor.commit();
		return true;
	}

	public void createBinary()  
	{ 
		copyBinary(R.raw.androidvncserver, getFilesDir().getAbsolutePath() + "/androidvncserver");
		copyBinary(R.raw.vncviewer, getFilesDir().getAbsolutePath()+"/VncViewer.jar");
		copyBinary(R.raw.indexvnc, getFilesDir().getAbsolutePath()+"/index.vnc");

		Process sh;
		try {
			sh = Runtime.getRuntime().exec("su");

			OutputStream os = sh.getOutputStream();

			writeCommand(os, "killall androidvncserver");
			writeCommand(os, "killall -KILL androidvncserver");			
			//chmod 777 SHOULD exist
			writeCommand(os, "chmod 777 " + getFilesDir().getAbsolutePath() + "/androidvncserver");
			os.close();
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
