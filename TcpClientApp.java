package application;

import com.kuka.roboticsAPI.applicationModel.RoboticsAPIApplication;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;

public class TcpClientApp extends RoboticsAPIApplication {

  private Socket socket;
  //private static final int SOCKET_TIMEOUT = 2000;

  private String hostIpAddress = "172.31.1.220";
  private int hostPort = 30001;

  @Override
  public void initialize() {
    // initialize your application here
  }

  @Override
  public void run() {
    getLogger().info("creating socket");
    initTCP();
    getLogger().info("Sending message");
    pingTCP(2000);
    getLogger().info("Client is shutting down.");

  }

 

private void initTCP() {
    // your application execution starts here
    try {
      socket = new Socket(hostIpAddress, hostPort);
     // socket.setSoTimeout(SOCKET_TIMEOUT);

    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }

  /**
   * Send an ping message to the host application/task.
   * 
   * @param timeout
   *        Timeout in Milliseconds to wait for an answer.
   */
  public void pingTCP(int timeout) {

    String fullMsgString = "PING";
    // ends msg stream with char
    String msg = fullMsgString.concat("\n");

    try {
      OutputStream os = socket.getOutputStream();
      OutputStreamWriter writer = new OutputStreamWriter(os);
      BufferedWriter bw = new BufferedWriter(writer);
      bw.write(msg);
      bw.flush();
    } catch (Exception e) {
      getLogger().info("No connection to server... " + e.getMessage());
    }
  }

}
