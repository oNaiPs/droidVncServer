package org.onaips.vnc;

import java.io.FileOutputStream;
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
		if (firstRun()) 
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
		if (Build.VERSION.SDK_INT>Build.VERSION_CODES.FROYO)
			copyBinary(R.raw.androidvncserver_gingerup , getFilesDir().getAbsolutePath() + "/androidvncserver");
		else 
			copyBinary(R.raw.androidvncserver_froyo , getFilesDir().getAbsolutePath() + "/androidvncserver");
		copyBinary(R.raw.vncviewer, getFilesDir().getAbsolutePath()+"/VncViewer.jar");
		copyBinary(R.raw.index, getFilesDir().getAbsolutePath()+"/index.vnc");

		copyBinary(R.raw.base64, getFilesDir().getAbsolutePath()+"/base64.js");
		copyBinary(R.raw.black, getFilesDir().getAbsolutePath()+"/black.css");
		copyBinary(R.raw.des, getFilesDir().getAbsolutePath()+"/des.js");
		copyBinary(R.raw.display, getFilesDir().getAbsolutePath()+"/display.js");
		copyBinary(R.raw.input, getFilesDir().getAbsolutePath()+"/input.js");
		copyBinary(R.raw.logo, getFilesDir().getAbsolutePath()+"/logo.js");
		copyBinary(R.raw.novnc, getFilesDir().getAbsolutePath()+"/novnc.html");
		copyBinary(R.raw.plain, getFilesDir().getAbsolutePath()+"/plain.css");
		copyBinary(R.raw.playback, getFilesDir().getAbsolutePath()+"/playback.js");
		copyBinary(R.raw.rfb, getFilesDir().getAbsolutePath()+"/rfb.js");
		copyBinary(R.raw.self, getFilesDir().getAbsolutePath()+"/self.pem");
		copyBinary(R.raw.swfobject, getFilesDir().getAbsolutePath()+"/swfobject.js");
		copyBinary(R.raw.ui, getFilesDir().getAbsolutePath()+"/ui.js.vnc");
		copyBinary(R.raw.util, getFilesDir().getAbsolutePath()+"/util.js");
		copyBinary(R.raw.vnc, getFilesDir().getAbsolutePath()+"/vnc.js");
		copyBinary(R.raw.web_socket, getFilesDir().getAbsolutePath()+"/web_socket.js");
		copyBinary(R.raw.websock, getFilesDir().getAbsolutePath()+"/websock.js");
		copyBinary(R.raw.websocketmain, getFilesDir().getAbsolutePath()+"/websocketmain.swf");
		copyBinary(R.raw.webutil, getFilesDir().getAbsolutePath()+"/webutil.js");
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
			log("public void createBinary(): " + e.getMessage());
		}


	}  

	static void writeCommand(OutputStream os, String command) throws Exception
	{
		os.write((command + "\n").getBytes("ASCII"));
	} 


}
