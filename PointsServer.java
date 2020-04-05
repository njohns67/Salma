package pointsServer;

import static com.kuka.roboticsAPI.motionModel.BasicMotions.ptp;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;

import org.json.JSONArray;
import org.json.JSONObject;
import com.kuka.roboticsAPI.applicationModel.RoboticsAPIApplication;
import com.kuka.roboticsAPI.deviceModel.LBR;
import com.kuka.roboticsAPI.geometricModel.Frame;

public class PointsServer extends RoboticsAPIApplication{
	//private BlockingQueue<JSONObject> queue = new LinkedBlockingQueue<JSONObject>();
	private LBR lbr; 
    public final static int port = 30005;
    Socket socket = null;
    int bytesRead;
    int current = 0;
    ServerSocket servSocket = null;
    JSONObject jsonFile = null;
	
	public void initialize() {		
		lbr = getContext().getDeviceFromType(LBR.class);
	}

	public void startServer() throws Exception{
		try{
            servSocket = new ServerSocket();
            servSocket.setReuseAddress(true);
            servSocket.bind(new InetSocketAddress(port));
	            System.out.println("Waiting...");
                InetAddress inetAddress = InetAddress.getLocalHost();
                System.out.println("IP Address: " + servSocket.getInetAddress());
                System.out.println("Host name: " + inetAddress.getHostName());
                try {
                    socket = servSocket.accept();
                    System.out.println("Accepted connection: " + socket);

                    InputStream is = socket.getInputStream();
                    BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    OutputStreamWriter writer = new OutputStreamWriter(socket.getOutputStream());
                    BufferedWriter bw = new BufferedWriter(writer);
                    
                    byte[] size = new byte[4];
                    int num = is.read(size);
                    int fileSize = ByteBuffer.wrap(size).getInt();
                    System.out.println("Size: " + fileSize + "\n" + num + " bytes read");

                    byte[] newFile = new byte[fileSize];
                    bytesRead = is.read(newFile, 0, newFile.length);
                    System.out.println("File downloaded (" + fileSize + " bytes read)");
                    jsonFile = new JSONObject(new String(newFile));
                    System.out.println(jsonFile);
            		if(jsonFile == null)
            			return;
            		JSONArray points = (JSONArray)jsonFile.get("points");
            		for(int i=0; i<points.length(); i++){
            			JSONObject point = points.getJSONObject(i);
            			System.out.println(point);
            			Double x = new Double(point.get("x").toString());
            			Double y = new Double(point.get("y").toString());
            			Double z = new Double(point.get("z").toString());
            			
                        Frame curPose = lbr.getCurrentCartesianPosition(lbr.getFlange(),getApplicationData().getFrame("/Virtual_PET"));
                        
                        curPose.setX(x);
                        curPose.setY(y);
                        curPose.setZ(z);
                        if(point.length() > 3){
            				Double a = new Double(point.get("a").toString());
            				Double b = new Double(point.get("b").toString());
            				Double g = new Double(point.get("g").toString());
                        	curPose.setAlphaRad(a);
                        	curPose.setBetaRad(b);
                        	curPose.setGammaRad(g);
                        }
                        lbr.move(ptp(curPose).setJointVelocityRel(.2));
                        System.out.println("Moved");
                        bw.write("Moved");
                        bw.flush();
                        String s = br.readLine();
                        System.out.println("Data was collected. Moving to next position");	
            		}
            		bw.write("Done");
            		bw.flush();
            	}
                finally {
                    System.out.println("Finished");
                    if(socket != null) socket.close();
                }
           // }
			}
			catch (IOException e) {
	        	System.out.println(e.getMessage());
				e.printStackTrace();
			}
	        finally{
	            System.out.println("Done");
	            if(servSocket != null)
					try {
						servSocket.close();
					} catch (IOException e) {
						System.out.println("Unable to close socket");
						e.printStackTrace();
					}
	        }
	}
	@Override
	public void run() throws Exception {	
		startServer();
	}
	
	public void dispose(){
		try {
			servSocket.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}

