package org.onaips.vnc_donate;

import java.io.IOException;
import java.io.OutputStream;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.util.Log;

public class StartAtBootService extends Service {

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return null;
	}
	

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) 
    {
    	startServer();
        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }


    public void startServer()
	{
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);

		//Lets see if i need to boot daemon...
		String startOnBoot=preferences.getString("startonboot", "Off");
		if (startOnBoot.equals("Off"))
			return;
    	
		
		//this code is redundant, how to merge it?
		try{					
			Process sh; 

			String password=preferences.getString("password", "");
			String password_check="";
			if (!password.equals(""))
				password_check="-p " + password;


			String rotation=preferences.getString("rotation", "0");
			rotation="-r " + rotation;

			String scaling=preferences.getString("scale", "100");
			
			String scaling_string="";
			if (!scaling.equals("0"))
				scaling_string="-s " + scaling;

			String port=preferences.getString("port", "5901");
			try
			{
				int port1=Integer.parseInt(port);
				port=String.valueOf(port1);
			}
			catch(NumberFormatException e)
			{
				port="5901";
			}
			String port_string="-P " + port;


			sh = Runtime.getRuntime().exec("su");
			OutputStream os = sh.getOutputStream();
			writeCommand(os, getFilesDir().getAbsolutePath() + "/androidvncserver "+ password_check + " " + rotation + " " + scaling_string + " " + port_string);

		} catch (IOException e) {
			Log.v("VNC","startServer():" + e.getMessage());
		} catch (Exception e) {
			Log.v("VNC","startServer():" + e.getMessage());
		}	

	}
    
	static void writeCommand(OutputStream os, String command) throws Exception
	{
		os.write((command + "\n").getBytes("ASCII"));
	} 
}
