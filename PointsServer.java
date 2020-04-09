package pointsServer;

import static com.kuka.roboticsAPI.motionModel.BasicMotions.ptp;
//import java.util.concurrent.BlockingQueue;
//import java.util.concurrent.LinkedBlockingQueue;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.lang.Thread;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;

import javax.inject.Inject;
import javax.inject.Named;

import org.json.JSONArray;
import org.json.JSONObject;
import com.kuka.roboticsAPI.applicationModel.RoboticsAPIApplication;
import com.kuka.roboticsAPI.deviceModel.LBR;
import com.kuka.roboticsAPI.geometricModel.Frame;
import com.kuka.roboticsAPI.geometricModel.ObjectFrame;
import com.kuka.roboticsAPI.geometricModel.Tool;
import com.kuka.roboticsAPI.geometricModel.math.Transformation;

public class PointsServer extends RoboticsAPIApplication{
	//private BlockingQueue<JSONObject> queue = new LinkedBlockingQueue<JSONObject>();
	private LBR lbr; 
	
	@Inject
	@Named("EA_SourceHolder")
	private Tool sourceholderTool;
	
    public final static int port = 30005;
    Socket socket = null;
    int bytesRead;
    int current = 0;
    ServerSocket servSocket = null;
    JSONObject jsonFile = null;
	
	public void initialize() {		
		lbr = getContext().getDeviceFromType(LBR.class);
	}

	public void writePosition(Frame curPos){
		try {
			FileWriter fw = new FileWriter("C:\\Users\\KukaUser\\Desktop\\positions.csv", true);
			BufferedWriter bout = new BufferedWriter(fw);
			Double x = curPos.getX();
			Double y = curPos.getY();
			Double z = curPos.getZ();
			Double a = curPos.getAlphaRad();
			Double b = curPos.getBetaRad();
			Double g = curPos.getGammaRad();
			bout.newLine();
			bout.write(String.format("%f, %f, %f, %f, %f, %f", x, y, z, a, b, g));
		    bout.close();
		    fw.close();
		} 
		catch (IOException e) {
		    System.out.println("An error occurred while writing to the file.");
		    e.printStackTrace();
		}
	}
	public void createPositionFile(){
		try {
            File myObj = new File("C:\\Users\\KukaUser\\Desktop\\positions.csv");
            if (myObj.createNewFile()) {
            	System.out.println("File created: " + myObj.getName());
            } 
            else {
            	System.out.println("File already exists.");
            }
        } 
        catch (IOException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
        }
		try {
			FileWriter fw = new FileWriter("C:\\Users\\KukaUser\\Desktop\\positions.csv", false);
			fw.write("x, y, z, a, b, g");
		    fw.close();
		} 
		catch (IOException e) {
		    System.out.println("An error occurred while writing to the file.");
		    e.printStackTrace();
		}
	}
	public void startServer() throws Exception{
		try{
            servSocket = new ServerSocket();
            servSocket.setReuseAddress(true);
            servSocket.bind(new InetSocketAddress(port));
	            System.out.println("Waiting...");
                try {
                    socket = servSocket.accept();
                    System.out.println("Accepted connection: " + socket);
                    createPositionFile();
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

           			//ObjectFrame P1 = getApplicationData().getFrame("/New_Virtual_PET");
           			//Frame curPose = P1.copyWithRedundancy();
           			
            		for(int i=0; i<points.length(); i++){

              			ObjectFrame P1 = getApplicationData().getFrame("/New_Virtual_PET");
               			Frame curPose = P1.copyWithRedundancy();
            			
            			JSONObject point = points.getJSONObject(i);
            			System.out.println(point);
            			Double x_Coord = new Double(point.get("x").toString());
            			Double y_Coord = new Double(point.get("y").toString());
            			Double z_Coord = new Double(point.get("z").toString());
            			
            			//ObjectFrame P_Frame = getApplicationData().getFrame("/EA_PointSource_Tool");
            			//ObjectFrame P_Frame = getApplicationData().getFrame("/Virtual_PET");
            			//Frame curPose = P_Frame.copyWithRedundancy();
           
            			System.out.println("Moving");
 
            			//Frame curPose = lbr.getCurrentCartesianPosition(lbr.getFlange(),getApplicationData().getFrame("/Virtual_PET"));
                    
            			Double x = curPose.getX() + x_Coord;
            			Double y = curPose.getY() + y_Coord;
            			Double z = curPose.getZ() + z_Coord;     
                          			
                        curPose.setX(x);
                        curPose.setY(y);
                        curPose.setZ(z);
                        
                        System.out.println(x);
                        System.out.println(y);
                        System.out.println(z);  
                        
                        
                        
                        if(point.length() > 3){
            				Double a_Ang = new Double(point.get("a").toString());
            				Double b_Ang = new Double(point.get("b").toString());
            				Double g_Ang = new Double(point.get("g").toString());
                        	
            				Double a = curPose.getAlphaRad() + a_Ang;
            				Double b = curPose.getBetaRad() + b_Ang;
            				Double g = curPose.getGammaRad() + g_Ang;	
            				
            				curPose.setAlphaRad(a);
                        	curPose.setBetaRad(b);
                        	curPose.setGammaRad(g);
                        }

                       // lbr.move(ptp(curPose).setJointVelocityRel(0.05));
                		sourceholderTool.move(ptp(curPose).setJointVelocityRel(0.05));
                     
                        System.out.println("Moved");
                        bw.write("Moved");
                        bw.flush();
                        br.readLine();
                        writePosition(curPose);
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
		sourceholderTool.attachTo(lbr.getFlange());
		while(true)
			startServer();		
	}
	
	public void dispose(){
		if(servSocket != null){
			try {
				servSocket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}

