package org.onaips.vnc;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import java.io.DataOutputStream;

/**
 * Servicio mínimo para lanzar el binario androidvncserver en Android-x86.
 * Requiere root (su).
 */
public class ServerManager extends Service {
    private static final String TAG = "ServerManager";
    private Process suProcess;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "ServerManager creado");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "Iniciando servidor VNC...");

        try {
            suProcess = Runtime.getRuntime().exec("su");
            DataOutputStream os = new DataOutputStream(suProcess.getOutputStream());

            // Ruta donde debería estar el binario androidvncserver
            String binPath = getFilesDir().getParent() + "/lib/libandroidvncserver.so";

            // Comando de arranque (puerto 5901, pass 1234)
            String cmd = binPath + " -p 1234 -P 5901\n";

            os.writeBytes("chmod 755 " + binPath + "\n");
            os.writeBytes(cmd);
            os.flush();
            os.writeBytes("exit\n");
            os.flush();

            Log.i(TAG, "Servidor arrancado en puerto 5901 con contraseña 1234");

        } catch (Exception e) {
            Log.e(TAG, "Error al arrancar VNC", e);
        }

        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (suProcess != null) {
            suProcess.destroy();
        }
        Log.i(TAG, "Servidor VNC detenido");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null; // No usamos binding
    }
}

