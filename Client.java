import java.io.BufferedOutputStream;
import java.io.FileNotFoundException;
import java.lang.StringIndexOutOfBoundsException;
import java.util.Scanner;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.io.EOFException;
import java.io.File;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class Client{
    public final static int port = 8000;
    public final static String server = "172.31.1.147";
    public static String fileToSend = "";
    

    public static void main(String[] args) throws IOException{
        Scanner input = new Scanner(System.in);
        System.out.println("Enter file location: ");
        fileToSend = input.next();
        /*try{
            while(fileToSend.substring((fileToSend.length()-4), fileToSend.length()) != ".json"){
                System.out.println("Error: File must be a json file");
                System.out.println("Enter file location: ");
                fileToSend = input.next();
            }
        }
        catch (StringIndexOutOfBoundsException e){
            System.out.println("Error: File must have an extension");
            System.exit(-2);
        }*/
        FileInputStream fin = null;
        BufferedInputStream bin = null;
        OutputStream os = null;
        Socket socket = null;
        try{
            socket = new Socket(server, port);
            System.out.println("Connecting...");
            try{
                File file = new File(fileToSend);
                byte[] temp = new byte[(int)file.length()];
                fin = new FileInputStream(file);
                bin = new BufferedInputStream(fin);
                bin.read(temp, 0, temp.length);
                os = socket.getOutputStream();
                System.out.println("Sending " + fileToSend + " (" + temp.length + " bytes)");
                byte[] size = ByteBuffer.allocate(4).putInt(temp.length).array(); 
                os.write(size, 0, size.length);
                os.write(temp, 0, temp.length);
                os.flush();
                System.out.println("Done");
            }
            catch(FileNotFoundException e){
                System.out.println("File " + fileToSend + " not found");
                System.exit(1);
            }
            finally{
                if(bin != null) bin.close();
                if(os != null) os.close();
                if(socket != null) socket.close();
            }
        }
        finally{
            if(socket != null) socket.close();
        }
    }
}
