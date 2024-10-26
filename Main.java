import java.util.*;
import java.io.*;
import java.net.*;
import java.nio.charset.*;

public class Main
{
	public static ServerSocket server;
	public static int port = 18881;
	public static List<Socket> clients = new ArrayList<Socket>();
	private static void waitConnection()
	{
		try
		{
			while(true)
			{
				Socket client = server.accept();
				clients.add(client);
			}
		}
		catch (IOException e)
		{
			System.out.println("Client connection : " + e.getMessage());
		}
	}

	public static void main(String[] argv)
	{
		try
		{
			server = new ServerSocket(port);
			Thread newClientsAdder = new Thread(() -> {waitConnection();});
			newClientsAdder.start();

			while(true)
			{
				for (Socket client : clients)
				{
					OutputStream out = client.getOutputStream();
					String message = "Get!";
					byte [] asciiBytes = message.getBytes(StandardCharsets.US_ASCII);
					out.write(asciiBytes);
					out.flush();
				}
			}
		}
		catch (IOException e)
		{
			System.out.println(e.getMessage());
		}
	}
}
