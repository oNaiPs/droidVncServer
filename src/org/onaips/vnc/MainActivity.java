
package org.onaips.vnc;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.http.conn.util.InetAddressUtils;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.graphics.Color;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.text.Html;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.google.ads.AdRequest;
import com.google.ads.AdSize;
import com.google.ads.AdView;

public class MainActivity extends Activity 
{      
	private static final int MENU_QUIT = 0;
	private static final int MENU_HELP = 1;  
	private static final int MENU_ONAIPS = 2;
	private static final int MENU_SENDLOG = 3;
	private static final int MENU_REVERSE_CONNECTION = 4;

	static final int APP_ID = 123;
	static final String VNC_LOG ="VNCserver";
	
	private AdView adView = null;
	private ServerManager s = null;
	private Animation buttonAnimation=null;
	private SharedPreferences preferences;
	private AlertDialog startDialog;

	void doBindService() {
		bindService(new Intent(this, ServerManager.class), mConnection,
				Context.BIND_AUTO_CREATE);
	}

	@Override
	public void onResume()
	{
		IntentFilter i;
		i = new IntentFilter("org.onaips.vnc.ACTIVITY_UPDATE");
		ActivityUpdateReceiver receiver=new ActivityUpdateReceiver();
		registerReceiver(receiver, i);

		super.onResume();
	}


	public class ActivityUpdateReceiver extends BroadcastReceiver
	{
		@Override
		public void onReceive(Context context, Intent intent)//this method receives broadcast messages. Be sure to modify AndroidManifest.xml file in order to enable message receiving
		{
			try {
				Thread.sleep(2000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
					setStateLabels(ServerManager.isServerRunning());		
		}
	}


	private ServiceConnection mConnection = new ServiceConnection() {

		public void onServiceConnected(ComponentName className, IBinder binder) {
			s = ((ServerManager.MyBinder) binder).getService();
		}

		public void onServiceDisconnected(ComponentName className) {
			s = null;
		}
	};

	@Override  
	protected void onDestroy()
	{
		if (adView != null)
			adView.destroy();
		super.onDestroy();
		unregisterReceiver(mReceiver);
	}

	//rodar vnc com acc   


 

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);  

		requestWindowFeature(Window.FEATURE_NO_TITLE); 

		setContentView(R.layout.main);

		doBindService();


		// Initialize preferences
		preferences = PreferenceManager.getDefaultSharedPreferences(this);

		boolean root=preferences.getBoolean("asroot",true);
		if (!hasRootPermission() && root)
		{
			log("You do not have root permissions...!!!");
			startDialog = new AlertDialog.Builder(this).create();
			startDialog.setTitle("Cannot continue");
			startDialog.setMessage("You do not have root permissions.\nPlease root your phone first!\n\nDo you want to continue anyway?");
			startDialog.setIcon(R.drawable.icon);

			startDialog.setButton("Yes", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface arg0, int arg1) {
					Editor e=preferences.edit();
					e.putBoolean("asroot", false);
					e.commit();

					startDialog.dismiss();
				}
			});
			startDialog.setButton2("No", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface arg0, int arg1) {
					System.exit(0);
				}
			});
			startDialog.show();
		}

		showInitialScreen(false);
		setStateLabels(ServerManager.isServerRunning());

		// register wifi event receiver
		registerReceiver(mReceiver,  new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));

		boolean hidead=preferences.getBoolean("hidead", false);
		if (!hidead)
		{

			// Look up the AdView as a resource and load a request.
			adView = (AdView)this.findViewById(R.id.adView);
			adView.loadAd(new AdRequest());
		} 

		findViewById(R.id.Button01).setOnClickListener(new OnClickListener() {
			public void onClick(View arg0) {
				final Button b=(Button)findViewById(R.id.Button01);
				
				buttonAnimation=AnimationUtils.loadAnimation(MainActivity.this, R.anim.animation);
				buttonAnimation.setAnimationListener(new AnimationListener() {
					public void onAnimationEnd(Animation animation) {
						b.setEnabled(true);
						//b.setVisibility(View.INVISIBLE);
					}

					public void onAnimationRepeat(Animation animation) {
					}

					public void onAnimationStart(Animation animation) {
						b.setEnabled(false);

						if (ServerManager.isServerRunning())
							stopServer();
						else
							startServer();
					}
				});
				b.startAnimation(buttonAnimation);

				return;
			}
		}) ;
		findViewById(R.id.Button02).setOnClickListener(new OnClickListener() {
			public void onClick(View arg0) {
				restartServer();
				return;
			}
		}); 
	}

	
	
	public void log(String s)
	{
		Log.v(VNC_LOG,s);
	}



	public String packageVersion()
	{
		String version = "";
		try {
			PackageInfo pi = getPackageManager().getPackageInfo(getPackageName(), 0);
			version = pi.versionName;     
		} catch (NameNotFoundException e) {
			log("onOptionsItemSelected: "+ e.getMessage());
		};
		return version;
	}


	public void showInitialScreen(boolean forceShow)
	{
		// Initialize preferences
		preferences = PreferenceManager.getDefaultSharedPreferences(this );
		SharedPreferences.Editor editor = preferences.edit();

		String version=packageVersion();

		if ((!forceShow) && (version.equals(preferences.getString("version", ""))))
			return;

		editor.putString("version", version);
		editor.commit();

		startDialog = new AlertDialog.Builder(this).create();
		startDialog.setTitle("droid VNC server");
		startDialog.setMessage(Html.fromHtml("Welcome to droid VNC server version " + version + ".<br>This is beta software so please provide some feedback about your experience!<br><br>Best Regards, @oNaiPs"));
		startDialog.setIcon(R.drawable.icon);


		startDialog.setButton(AlertDialog.BUTTON1,"OK", new DialogInterface.OnClickListener() {

			public void onClick(DialogInterface arg0, int arg1) {
				startDialog.dismiss();			
			}
		});

		startDialog.setButton2("Donate", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface arg0, int arg1) {
				Intent myIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=org.onaips.donate"));
				startActivity(myIntent);

			} 
		});

		startDialog.show();

	}

	public void showTextOnScreen(final String t)
	{
		runOnUiThread(new Runnable(){
			public void run() {
				Toast.makeText(MainActivity.this,t,Toast.LENGTH_LONG).show();
			}
		});
	}

	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.menu, menu);

		menu.add(0, MENU_SENDLOG  ,0,"Report issue");
		menu.add(0, MENU_ONAIPS   ,0,"About");
		menu.add(0, MENU_HELP     ,0, "Help");
		menu.add(0, MENU_REVERSE_CONNECTION, 0, "Reverse\nConnection");
		menu.add(0, MENU_QUIT     ,0, "Close");


		return true; 
	}


	public void setStateLabels(boolean state)
	{

		
		TextView stateLabel=(TextView)findViewById(R.id.stateLabel);
		stateLabel.setText(state?"Running":"Stopped");


		stateLabel.setTextColor(state?Color.rgb(114,182,43):Color.rgb(234,113,29));

		TextView t=(TextView)findViewById(R.id.TextView01);
 
		Button b=(Button)findViewById(R.id.Button01);
		b.clearAnimation();
		Button b2=(Button)findViewById(R.id.Button02);
		if (state)
		{
			String port=preferences.getString("port", "5901");
			String httpport;
			try
			{
				int port1=Integer.parseInt(port);
				port=String.valueOf(port1);
				httpport=String.valueOf(port1-100);
			}
			catch(NumberFormatException e)
			{
				port="5901";
				httpport="5801";
			}

			String ip=getIpAddress();
			if (ip.equals(""))
				t.setText(Html.fromHtml("Not connected to a network.<br> You can connect through USB with:<br>localhost:" + port + "<br>or<br>http://localhost:" + httpport + "<br>(use adb to forward ports)</font>"));
			else
				t.setText(Html.fromHtml("<font align=\"center\">Connect to:<br>" + ip+":" + port + "<br>or<br>http://" + ip + ":" + httpport + "</font>"));	

			b.setBackgroundDrawable(getResources().getDrawable(R.drawable.btnstop_normal));
			b2.setVisibility(View.VISIBLE);

		} 
		else
		{
			t.setText("");
			b.setBackgroundDrawable(getResources().getDrawable(R.drawable.btnstart_normal));
			b2.setVisibility(View.INVISIBLE);
		}  

	} 

	public String getIpAddress() {
		try {
			String ipv4;
			for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
				NetworkInterface intf = en.nextElement();
				for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
					InetAddress inetAddress = enumIpAddr.nextElement();
					if (!inetAddress.isLoopbackAddress()) {
						if (!inetAddress.isLoopbackAddress() && InetAddressUtils.isIPv4Address(ipv4=inetAddress.getHostAddress())) 
							return ipv4;
					}
				}
			}
		} catch (SocketException ex) {
			log(ex.toString());
		}
		return "";
	}

	public void restartServer()
	{

		startDialog = new AlertDialog.Builder(this).create();
		startDialog.setTitle("Already running");
		startDialog.setMessage("Restart server?");
		startDialog.setButton(AlertDialog.BUTTON1,"Yes", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface arg0, int arg1) {
				stopServer();
				try {
					Thread.sleep(2000);
				} catch (InterruptedException e) {
				}
				startServer();
			}
		});

		startDialog.setButton2("No", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface arg0, int arg1) {
			} 
		});
		startDialog.show();

	}


	public void startServer()
	{
		s.startServer();
		Timer t=new Timer();
		t.schedule(new TimerTask() {

			@Override
			public void run() {
				if (!ServerManager.isServerRunning())
				{
					runOnUiThread(new Runnable() {

						public void run() {
							showTextOnScreen("Could not start server :(");
							log("Could not start server :(");
							setStateLabels(ServerManager.isServerRunning());
						}
					});
				} 
			}
		},2000);
	}
	public void stopServer()
	{
		s.killServer();
		Timer t=new Timer();
		t.schedule(new TimerTask() {

			@Override
			public void run() {
				if (ServerManager.isServerRunning())
				{
					runOnUiThread(new Runnable() {

						public void run() {
							showTextOnScreen("Could not stop server :(");
							log("Could not stop server :(");
							setStateLabels(ServerManager.isServerRunning());							
						}
					});
				}
			}
		},4000);
	}

	public void showHelp()
	{
		new AlertDialog.Builder(this)
		.setTitle("Help")
		.setMessage(Html.fromHtml("Mouse Mappings:<br><br>Right Click -> Back<br>Middle Click -> End Call<br>Left Click -> Touch<br><br>Keyboard Mappings<br><br>" +
				"Home Key -> Home<br>Escape -> Back<br>Page Up ->Menu<br>Left Ctrl -> Search<br>PgDown -> Start Call<br>" +
		"End Key -> End Call<br>F4 -> Rotate<br>F11 -> Disconnect<br>F12 -> Stop Server Daemon"))
		.setPositiveButton("Quit", null)
		.setNegativeButton("Open Website", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface arg0, int arg1) {
				Intent myIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://www.onaips.com"));
				startActivity(myIntent);
			}
		})
		.show();
	}


	// This method is called once the menu is selected
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		// We have only one menu option
		case R.id.preferences:
			// Launch Preference activity
			Intent i = new Intent(MainActivity.this, preferences.class);
			startActivity(i);

			showTextOnScreen("Don't forget to stop/start the server after changes");

			break; 
		case MENU_QUIT:
			System.exit(1);
			break;
		case MENU_HELP:
			showHelp();
			break;
		case MENU_REVERSE_CONNECTION:
			if (ServerManager.isServerRunning())
			{
				startDialog = new AlertDialog.Builder(this).create();
				startDialog.setTitle("Already running");
				startDialog.setMessage("Restart server?");
				startDialog.setButton(AlertDialog.BUTTON1,"Yes", new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface arg0, int arg1) {
						startReverseConnection();
					}
				});

				startDialog.setButton2("Cancel", new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface arg0, int arg1) {
					} 
				});
				startDialog.show();
			}
			else
				startReverseConnection();

			break;
		case MENU_SENDLOG:
			collectAndSendLog();
			break;
		case MENU_ONAIPS:

			new AlertDialog.Builder(this)
			.setTitle("About")
			.setMessage(Html.fromHtml("version " + packageVersion() + "<br><br>Code: @oNaiPs<br><br>Graphics: ricardomendes.net<br><br>Under the GPLv3"))
			.setPositiveButton("Close", null)
			.setNegativeButton("Open Website", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface arg0, int arg1) {
					Intent myIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://onaips.com"));
					startActivity(myIntent);

				}
			})
			.show();
			break;

		} 
		return true;  
	}

	public void startReverseConnection()
	{

		AlertDialog.Builder alert = new AlertDialog.Builder(this);

		alert.setTitle("Reverse Connection");
		alert.setMessage("Input <host:port>:");

		// Set an EditText view to get user input 
		final EditText input = new EditText(this);

		alert.setView(input);

		alert.setPositiveButton("Start server", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				s.startReverseConnection(input.getText().toString());
			}
		});

		alert.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				// Canceled.
			}
		});

		alert.show();
	}

	public static boolean hasRootPermission() {
		boolean rooted = true;
		try {
			File su = new File("/system/bin/su");
			if (su.exists() == false) {
				su = new File("/system/xbin/su");
				if (su.exists() == false) {
					rooted = false;
				}
			}
		} catch (Exception e) {
			//log( "Can't obtain root - Here is what I know: "+e.getMessage());
			rooted = false;
		}

		return rooted;
	}

	public static final String LOG_COLLECTOR_PACKAGE_NAME = "com.xtralogic.android.logcollector";
	public static final String ACTION_SEND_LOG = "com.xtralogic.logcollector.intent.action.SEND_LOG";
	public static final String EXTRA_SEND_INTENT_ACTION = "com.xtralogic.logcollector.intent.extra.SEND_INTENT_ACTION";
	public static final String EXTRA_DATA = "com.xtralogic.logcollector.intent.extra.DATA";
	public static final String EXTRA_ADDITIONAL_INFO = "com.xtralogic.logcollector.intent.extra.ADDITIONAL_INFO";
	public static final String EXTRA_SHOW_UI = "com.xtralogic.logcollector.intent.extra.SHOW_UI";
	public static final String EXTRA_FILTER_SPECS = "com.xtralogic.logcollector.intent.extra.FILTER_SPECS";
	public static final String EXTRA_FORMAT = "com.xtralogic.logcollector.intent.extra.FORMAT";
	public static final String EXTRA_BUFFER = "com.xtralogic.logcollector.intent.extra.BUFFER";

	void collectAndSendLog(){
		final PackageManager packageManager = getPackageManager();
		final Intent intent = new Intent(ACTION_SEND_LOG);
		List<ResolveInfo> list = packageManager.queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY);
		final boolean isInstalled = list.size() > 0;

		if (!isInstalled){
			new AlertDialog.Builder(this)
			.setTitle(getString(R.string.app_name))
			.setIcon(android.R.drawable.ic_dialog_info)
			.setMessage("Please install Log Collector application to collect the device log and send it to dev.")
			.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener(){
				public void onClick(DialogInterface dialog, int whichButton){
					Intent marketIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("market://search?q=pname:" + LOG_COLLECTOR_PACKAGE_NAME));
					marketIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					startActivity(marketIntent); 
				}
			})
			.setNegativeButton(android.R.string.cancel, null)
			.show();
		}
		else{
			new AlertDialog.Builder(this)
			.setTitle(getString(R.string.app_name))
			.setIcon(android.R.drawable.ic_dialog_info)
			.setMessage("Do you want to send a bug report to the dev? Please specify what problem is ocurring.\n\n" +
					"Make sure you started & stopped the server before submitting")
			.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener(){
				public void onClick(DialogInterface dialog, int whichButton){
					intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					intent.putExtra(EXTRA_SEND_INTENT_ACTION, Intent.ACTION_SENDTO);
					final String email = "onaips@gmail.com";
					intent.putExtra(EXTRA_DATA, Uri.parse("mailto:" + email));
					intent.putExtra(EXTRA_ADDITIONAL_INFO,"Problem Description: \n\n\n\n---------DEBUG--------\n" + 
					getString(R.string.device_info_fmt,getVersionNumber(getApplicationContext()),Build.MODEL,Build.VERSION.RELEASE, 
							getFormattedKernelVersion(), Build.DISPLAY,Build.CPU_ABI));
					intent.putExtra(Intent.EXTRA_SUBJECT, "droid VNC server: Debug Info");
					intent.putExtra(EXTRA_FORMAT, "time");

					//The log can be filtered to contain data relevant only to your app
					String[] filterSpecs = new String[4];
					filterSpecs[0] = VNC_LOG + ":I";
					filterSpecs[1] = VNC_LOG + ":D";
					filterSpecs[2] = VNC_LOG + ":V";
					filterSpecs[3] = "*:S";
					intent.putExtra(EXTRA_FILTER_SPECS, filterSpecs);

					startActivity(intent);
				}
			})
			.setNegativeButton(android.R.string.cancel, null)
			.show();
		}
	}

	private String getFormattedKernelVersion() 
	{
		String procVersionStr;

		try {
			BufferedReader reader = new BufferedReader(new FileReader("/proc/version"), 256);
			try {
				procVersionStr = reader.readLine();
			} finally {
				reader.close();
			}

			final String PROC_VERSION_REGEX =
				"\\w+\\s+" + /* ignore: Linux */
				"\\w+\\s+" + /* ignore: version */
				"([^\\s]+)\\s+" + /* group 1: 2.6.22-omap1 */
				"\\(([^\\s@]+(?:@[^\\s.]+)?)[^)]*\\)\\s+" + /* group 2: (xxxxxx@xxxxx.constant) */
				"\\([^)]+\\)\\s+" + /* ignore: (gcc ..) */
				"([^\\s]+)\\s+" + /* group 3: #26 */
				"(?:PREEMPT\\s+)?" + /* ignore: PREEMPT (optional) */
				"(.+)"; /* group 4: date */

			Pattern p = Pattern.compile(PROC_VERSION_REGEX);
			Matcher m = p.matcher(procVersionStr);

			if (!m.matches()) {
				log("Regex did not match on /proc/version: " + procVersionStr);
				return "Unavailable";
			} else if (m.groupCount() < 4) {
				log("Regex match on /proc/version only returned " + m.groupCount()
						+ " groups");
				return "Unavailable";
			} else {
				return (new StringBuilder(m.group(1)).append("\n").append(
						m.group(2)).append(" ").append(m.group(3)).append("\n")
						.append(m.group(4))).toString();
			}
		} catch (IOException e) {  
			log("IO Exception when getting kernel version for Device Info screen"+ e.getMessage() );

			return "Unavailable";
		}
	}

	private static String getVersionNumber(Context context) 
	{
		String version = "?";
		try 
		{
			PackageInfo packagInfo = context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
			version = packagInfo.versionName;
		} 
		catch (PackageManager.NameNotFoundException e){};

		return version;
	}


	static void writeCommand(OutputStream os, String command) throws Exception
	{
		os.write((command + "\n").getBytes("ASCII"));
	}

	public BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override public void onReceive(Context context, Intent intent) {
			NetworkInfo info = intent.getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
			if (info.getType() == ConnectivityManager.TYPE_MOBILE || info.getType()==ConnectivityManager.TYPE_WIFI) {
				setStateLabels(ServerManager.isServerRunning());

			}  
		}
	};

	static File findExecutableOnPath(String executableName)  
	{  

		String systemPath = System.getenv("PATH");  
		String[] pathDirs = systemPath.split(File.pathSeparator);  

		File fullyQualifiedExecutable = null;  
		for (String pathDir : pathDirs)  
		{  
			File file = new File(pathDir, executableName);  
			if (file.isFile())  
			{  
				fullyQualifiedExecutable = file;  
				break;  
			}  
		}  
		return fullyQualifiedExecutable;  
	}
}