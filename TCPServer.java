package application;

import java.io.BufferedReader;
import java.io.Console;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.logging.Logger;

public class TCPServer {

	/**
     * Members for socket communication with the external GUI (e.g. ExampleLBRGUI)
     */
    private static InetSocketAddress _externalClientAddress = null;
    private static ServerSocket _socket;
    private static Socket sc;
 
    private int _localPort;
    private static final int SOCKET_TIMEOUT = 20;
    private static final int localPort = 30001;
	
	private static final Logger log = Logger.getLogger(TCPServer.class.getName());
    
	/**
	 * @param args
	 */
	public static void main(String[] args) {
	
		log.info("Server is starting... ");

		initTCP();
		connect();
		communicate();
		
		log.info("Server shutting down... ");

	}
	
	
	private static void communicate() {
		boolean isRec = false;
		while (!isRec)
		{
			isRec = receive();
		}
		
		log.info("Receive loop exited... ");
	}


	private static void connect() 
	{
		log.info("Waiting for client conneciton... ");
		
		while (null == _externalClientAddress) 
		{
			try {
				
				sc = _socket.accept();
				_externalClientAddress = new InetSocketAddress(sc.getInetAddress(), sc.getPort());
				
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
		
	}


	private static void initTCP() 
	{
		try {
			_socket = new ServerSocket();
			_socket.setReuseAddress(true);
			_socket.bind(new InetSocketAddress(localPort));
			
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

	private static boolean receive()
	{
	    //byte[] receivedData = new byte[TCP_PACKET_SIZE];
		BufferedReader br;
		try {
			
			br = new BufferedReader( 
					new InputStreamReader(sc.getInputStream()));
			
			String msg = br.readLine();
			
			if (msg.length() != 0)
	        {
				log.info("Message recieved " + msg);
				return true;
	        }   
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return false;
	}
	
	private void dispose()
	{
		try {
			_socket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
}
