package org.onaips.vnc;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Application;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.preference.PreferenceManager;
import android.util.Log;

public class MainApplication extends Application {

 

	@Override 
	public void onCreate() {
		super.onCreate(); 
		//if (firstRun()) 
			createBinaries();
	}    
 
	public void log(String s)
	{ 
		Log.v(MainActivity.VNC_LOG,s); 
	}

	public boolean firstRun()
	{
		int versionCode = 0;
		try {
			versionCode = getPackageManager()
			.getPackageInfo(getPackageName(), PackageManager.GET_META_DATA)
			.versionCode;
		} catch (NameNotFoundException e) {
			log("Package not found... Odd, since we're in that package..." + e.getMessage());
		}

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		int lastFirstRun = prefs.getInt("last_run", 0);

		if (lastFirstRun >= versionCode) {
			log("Not first run");
			return false; 
		}
		log("First run for version " + versionCode); 

		SharedPreferences.Editor editor = prefs.edit();
		editor.putInt("last_run", versionCode);
		editor.commit(); 
		return true;
	}   
 
	public void createBinaries()  
	{   
		String filesdir = getFilesDir().getAbsolutePath()+"/";
 
		//copy html related stuff
		copyBinary(R.raw.webclients, filesdir + "/webclients.zip");
		 
		try {
			ResLoader.unpackResources(R.raw.webclients, getApplicationContext(),filesdir);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
	}

	public void copyBinary(int id,String path)
	{
		log("copy -> " + path);
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
			log("public void createBinary() error! : " + e.getMessage());
		}
	}  

	static void writeCommand(OutputStream os, String command) throws Exception
	{
		os.write((command + "\n").getBytes("ASCII"));
	} 
}
