package org.onaips.vnc;

import android.app.Activity;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentSender.SendIntentException;
import android.util.Log;

public class DaemonCommunication extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();

		Log.v("VNC","daemoncom" + action);

		if (action.equalsIgnoreCase("org.onaips.vnc.intent.action.DaemonCommunication.ClientConnected"))
		{
			//	String ns = Context.NOTIFICATION_SERVICE;
			//NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
			Log.v("VNC","daemoncom");
			Intent i = new Intent("org.onaips.vnc.CLIENTCONNECTED");
			context.sendOrderedBroadcast(i, null, new BroadcastReceiver() {
				@Override
				public void onReceive(Context context, Intent intent) {
					int result = getResultCode();
					if (result != Activity.RESULT_CANCELED) {
						Log.d("VNC", "Activity caught the broadcast, result "+result);
						return;  // Activity caught it
					}
					Log.d("VNC", "Activity did not catch the broadcast");
				}
			}, null, Activity.RESULT_CANCELED, null, null);		}
		else if (action.equalsIgnoreCase("org.onaips.vnc.intent.action.DaemonCommunication.ClientDisconnected"))
		{
			Log.v("VNC","daemoncom");
			Intent i = new Intent("org.onaips.vnc.CLIENTDISCONNECTED");
			context.sendOrderedBroadcast(i, null, new BroadcastReceiver() {
				@Override
				public void onReceive(Context context, Intent intent) {
					int result = getResultCode();
					if (result != Activity.RESULT_CANCELED) {
						Log.d("VNC", "Activity caught the broadcast, result "+result);
						return;  // Activity caught it
					}
					Log.d("VNC", "Activity did not catch the broadcast");
				}
			}, null, Activity.RESULT_CANCELED, null, null);		
		}
	}
}


