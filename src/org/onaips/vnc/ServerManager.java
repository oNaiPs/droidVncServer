package org.onaips.vnc;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.text.ClipboardManager;
import android.util.Log;
import android.widget.Toast;

public class ServerManager extends Service {
	SharedPreferences preferences;
	private static PowerManager.WakeLock wakeLock = null;

	boolean serverOn = false;
	public static String SOCKET_ADDRESS = "org.onaips.vnc.gui";
	SocketListener serverConnection = null;

	private String rHost = null;
	private final IBinder mBinder = new MyBinder();
	private Handler handler;

	@Override
	public void onCreate() {
		super.onCreate();

		handler = new Handler(Looper.getMainLooper());
		preferences = PreferenceManager.getDefaultSharedPreferences(this);

		if (serverConnection != null) {
			log("ServerConnection was already active!");
		} else {
			log("ServerConnection started");
			serverConnection = new SocketListener();
			serverConnection.start();
		}

	}
 

	//for pre-2.0 devices
	@Override
	public void onStart(Intent intent, int startId) {
		handleStart();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		handleStart();
		return START_NOT_STICKY;
	}
	
	private void handleStart()
	{
		log("ServerManager::handleStart");

		Boolean startdaemon = preferences.getBoolean("startdaemononboot",
				false);
		log("Let me see if we need to start daemon..."
				+ (startdaemon ? "Yes" : "No"));
		if (startdaemon)
			startServer();		
	}

	public void startServer() {
		// Lets see if i need to boot daemon...
		try {
			Process sh;
			String files_dir = getFilesDir().getAbsolutePath();

			String password = preferences.getString("password", "");
			String password_check = "";
			if (!password.equals(""))
				password_check = "-p " + password;

			String rotation = preferences.getString("rotation", "0");
			if (!rotation.equals(""))
				rotation = "-r " + rotation;

			String scaling = preferences.getString("scale", "100");

			String scaling_string = "";
			if (!scaling.equals(""))
				scaling_string = "-s " + scaling;

			String port = preferences.getString("port", "5901");
			try {
				int port1 = Integer.parseInt(port);
				port = String.valueOf(port1);
			} catch (NumberFormatException e) {
				port = "5901";
			}
			String port_string = "";
			if (!port.equals(""))
				port_string = "-P " + port;

			String reverse_string = "";
			if (rHost != null && !rHost.equals(""))
				reverse_string = "-R " + rHost;


			String display_method = "";
			if (!preferences.getString("displaymode", "auto").equals("auto"))
				display_method = "-m " + preferences.getString("displaymode", "auto");

			String display_zte="";
			if (preferences.getBoolean("rotate_zte", false))
				display_zte = "-z";
			
			//our exec file is disguised as a library so it will get packed to lib folder according to cpu_abi
			String droidvncserver_exec=getFilesDir().getParent() + "/lib/libandroidvncserver.so";
			File f=new File (droidvncserver_exec);
			if (!f.exists())
			{
				String e="Error! Could not find daemon file, " + droidvncserver_exec;
				showTextOnScreen(e);
				log(e);
				return;
			}
			
			
			Runtime.getRuntime().exec("chmod 777 " + droidvncserver_exec);
 
			String permission_string="chmod 777 " + droidvncserver_exec;
			String server_string= droidvncserver_exec  + " " + password_check + " " + rotation+ " " + scaling_string + " " + port_string + " "
			+ reverse_string + " " + display_method + " " + display_zte;
 
			boolean root=preferences.getBoolean("asroot",true);
			root &= MainActivity.hasRootPermission();
 
			if (root)     
			{ 
				log("Running as root...");
				sh = Runtime.getRuntime().exec("su",null,new File(files_dir));
				OutputStream os = sh.getOutputStream();
				writeCommand(os, permission_string);
				writeCommand(os, server_string);
			}
			else
			{
				log("Not running as root...");
				Runtime.getRuntime().exec(permission_string);
				Runtime.getRuntime().exec(server_string,null,new File(files_dir));
			}
			// dont show password on logcat
			log("Starting " + droidvncserver_exec  + " " + rotation+ " " + scaling_string + " " + port_string + " "
					+ reverse_string + " " + display_method + " " + display_zte);

		} catch (IOException e) {
			log("startServer():" + e.getMessage());
		} catch (Exception e) {
			log("startServer():" + e.getMessage());
		}

	}

	void startReverseConnection(String host) {
		try {
			rHost = host;

			if (isServerRunning()) {
				killServer();
				Thread.sleep(2000);
			}

			startServer();
			rHost = null;

		} catch (InterruptedException e) {
			log(e.getMessage());
		}
	}

	void killServer() {
		try {
			DatagramSocket clientSocket = new DatagramSocket();
			InetAddress addr = InetAddress.getLocalHost();
			String toSend = "~KILL|";
			byte[] buffer = toSend.getBytes();

			DatagramPacket question = new DatagramPacket(buffer, buffer.length,
					addr, 13132);
			clientSocket.send(question);
		} catch (Exception e) {

		}
	}

	
	
	
	public static boolean isServerRunning() {
		try {
			byte[] receiveData = new byte[1024];
			DatagramSocket clientSocket = new DatagramSocket();
			InetAddress addr = InetAddress.getLocalHost();

			clientSocket.setSoTimeout(100);
			String toSend = "~PING|";
			byte[] buffer = toSend.getBytes();

			DatagramPacket question = new DatagramPacket(buffer, buffer.length,
					addr, 13132);
			clientSocket.send(question);

			DatagramPacket receivePacket = new DatagramPacket(receiveData,
					receiveData.length);
			clientSocket.receive(receivePacket);
			String receivedString = new String(receivePacket.getData());
			receivedString = receivedString.substring(0, receivePacket
					.getLength());

			return receivedString.equals("~PONG|");
		} catch (Exception e) {
			return false;
		}
	}

	class SocketListener extends Thread {
		DatagramSocket server = null;
		boolean finished = false;

		public void finishThread() {
			finished = true;
		}

		@Override
		public void run() {
			try {
				server = new DatagramSocket(13131);
				log("Listening...");

				while (!finished) {
					DatagramPacket answer = new DatagramPacket(new byte[1024],
							1024);
					server.receive(answer);

					String resp = new String(answer.getData());
					resp = resp.substring(0, answer.getLength());

					log("RECEIVED " + resp);  

					if (resp.length() > 5
							&& resp.substring(0, 6).equals("~CLIP|")) {
						resp = resp.substring(7, resp.length() - 1);
						ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);

						clipboard.setText(resp.toString());
					} else if (resp.length() > 6
							&& resp.substring(0, 6).equals("~SHOW|")) {
						resp = resp.substring(6, resp.length() - 1);
						showTextOnScreen(resp);
					} else if (resp.length() > 15
							&& (resp.substring(0, 15).equals("~SERVERSTARTED|") || resp
									.substring(0, 15).equals("~SERVERSTOPPED|"))) {
						Intent intent = new Intent("org.onaips.vnc.ACTIVITY_UPDATE");
						sendBroadcast(intent);
					} 
					else if (preferences.getBoolean("notifyclient", true)) {
						if (resp.length() > 10
								&& resp.substring(0, 11).equals("~CONNECTED|")) {
							resp = resp.substring(11, resp.length() - 1);
							showClientConnected(resp);
						} else if (resp.length() > 13
								&& resp.substring(0, 14).equals(
								"~DISCONNECTED|")) {
							showClientDisconnected();
						}
					} else {
						log("Received: " + resp);
					}
				}
			} catch (IOException e) {
				log("ERROR em SOCKETLISTEN " + e.getMessage());
			}
		}
	}

	public void showClientConnected(String c) {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);

		int icon = R.drawable.icon;
		CharSequence tickerText = c + " connected to VNC server";
		long when = System.currentTimeMillis();

		Notification notification = new Notification(icon, tickerText, when);

		Context context = getApplicationContext();
		CharSequence contentTitle = "Droid VNC Server";
		CharSequence contentText = "Client Connected from " + c;
		Intent notificationIntent = new Intent();
		PendingIntent contentIntent = PendingIntent.getActivity(
				getApplicationContext(), 0, notificationIntent, 0);

		notification.setLatestEventInfo(context, contentTitle, contentText,
				contentIntent);

		mNotificationManager.notify(MainActivity.APP_ID, notification);

		// lets see if we should keep screen on
		if (preferences.getBoolean("screenturnoff", false)) {
			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
			wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "VNC");
			wakeLock.acquire();
		}
	}

	void showClientDisconnected() {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
		mNotificationManager.cancel(MainActivity.APP_ID);

		if (wakeLock != null && wakeLock.isHeld())
			wakeLock.release();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		//showTextOnScreen("Droid VNC server service killed...");
	}

	static void writeCommand(OutputStream os, String command) throws Exception {
		os.write((command + "\n").getBytes("ASCII"));
	}

	public void showTextOnScreen(final String t) {
		handler.post(new Runnable() {

			public void run() {
				// TODO Auto-generated method stub
				Toast.makeText(getApplicationContext(), t, Toast.LENGTH_LONG)
				.show();
			}
		});
	}

	public void log(String s) {
		Log.v(MainActivity.VNC_LOG, s);
	}


	// We return the binder class upon a call of bindService
	@Override
	public IBinder onBind(Intent arg0) {
		return mBinder;
	}

	public class MyBinder extends Binder {
		ServerManager getService() {
			return ServerManager.this;
		}
	}
}
