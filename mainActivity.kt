package com.themes.esp

import android.Manifest
import android.bluetooth.*
import android.content.pm.PackageManager
import android.os.Bundle
import android.speech.tts.TextToSpeech
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import kotlinx.coroutines.*
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.OutputStream
import java.net.HttpURLConnection
import java.net.URL
import java.util.*

class MainActivity : AppCompatActivity(), TextToSpeech.OnInitListener {

    private lateinit var btnConnect: Button
    private lateinit var txtReceived: TextView
    private lateinit var txtDebug: TextView

    private lateinit var tts: TextToSpeech
    private var bluetoothAdapter: BluetoothAdapter? = null
    private var socket: BluetoothSocket? = null
    private var inputStream: BufferedReader? = null
    private var outputStream: OutputStream? = null

    private val espName = "ESP32_BT"
    private val postURL = "http://your-server.com/api"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        btnConnect = findViewById(R.id.btnConnect)
        txtReceived = findViewById(R.id.txtReceived)
        txtDebug = findViewById(R.id.txtDebug)

        tts = TextToSpeech(this, this)
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()

        btnConnect.setOnClickListener {
            txtDebug.text = "Connecting..."
            connectToESP()
        }
    }

    private fun connectToESP() {
        val adapter = bluetoothAdapter ?: return showError("Bluetooth not supported")

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
            != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(this,
                arrayOf(Manifest.permission.BLUETOOTH_CONNECT), 100)
            return
        }

        val device = adapter.bondedDevices.find { it.name == espName }
        if (device == null) return showError("ESP32 not paired")

        CoroutineScope(Dispatchers.IO).launch {
            try {
                val uuid = device.uuids[0].uuid
                socket = device.createRfcommSocketToServiceRecord(uuid)
                socket?.connect()

                inputStream = BufferedReader(InputStreamReader(socket!!.inputStream))
                outputStream = socket!!.outputStream

                withContext(Dispatchers.Main) {
                    txtDebug.text = "Connected ✅"
                }

                readLoop()

            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    showError("Connect failed: ${e.message}")
                }
            }
        }
    }

    private suspend fun readLoop() {
        try {
            while (true) {
                val msg = inputStream?.readLine() ?: break

                withContext(Dispatchers.Main) {
                    txtReceived.text = "Received: $msg"
                }

                sendPost(msg)
            }
        } catch (e: Exception) {
            withContext(Dispatchers.Main) { showError("Disconnected: ${e.message}") }
        }
    }

    private fun sendPost(data: String) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                val url = URL(postURL)
                val conn = url.openConnection() as HttpURLConnection

                conn.requestMethod = "POST"
                conn.setRequestProperty("Content-Type", "application/json")
                conn.doOutput = true

                val json = """{"value":"$data"}"""
                conn.outputStream.write(json.toByteArray())

                val response = conn.inputStream.bufferedReader().readText()

                withContext(Dispatchers.Main) {
                    speak(response)
                }

            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    showError("POST failed: ${e.message}")
                }
            }
        }
    }

    override fun onInit(status: Int) {
        if (status == TextToSpeech.SUCCESS) tts.language = Locale.US
    }

    private fun speak(text: String) {
        try {
            tts.speak(text, TextToSpeech.QUEUE_FLUSH, null, null)
        } catch (e: Exception) {
            showError("TTS error: ${e.message}")
        }
    }

    private fun showError(msg: String) {
        txtDebug.text = "Error: $msg"
    }

    override fun onDestroy() {
        super.onDestroy()
        try { socket?.close() } catch (_: Exception) {}
        tts.shutdown()
    }
}
