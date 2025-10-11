package org.onaips.vnc;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;

/**
 * MainActivity mínima para Android-x86.
 * Permite arrancar y parar el servicio ServerManager (VNC).
 */
public class MainActivity extends Activity {

    private Intent serverIntent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        serverIntent = new Intent(this, ServerManager.class);

        // Creamos layout básico en código (sin XML)
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(50, 50, 50, 50);

        // Botón Start
        Button startButton = new Button(this);
        startButton.setText("Start VNC Server");
        startButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startService(serverIntent);
                Toast.makeText(MainActivity.this,
                        "VNC Server iniciado en puerto 5901 (pass 1234)",
                        Toast.LENGTH_LONG).show();
            }
        });

        // Botón Stop
        Button stopButton = new Button(this);
        stopButton.setText("Stop VNC Server");
        stopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                stopService(serverIntent);
                Toast.makeText(MainActivity.this,
                        "VNC Server detenido",
                        Toast.LENGTH_LONG).show();
            }
        });

        layout.addView(startButton);
        layout.addView(stopButton);

        setContentView(layout);
    }
}
