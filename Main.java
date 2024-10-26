import java.util.*;
import java.io.*;
import java.net.*;
import java.nio.charset.*;

public class Main
{
	public static ServerSocket server;
	public static int port = 18881;
	public static List<User> clients = new ArrayList<User>();
	private static void waitConnection()
	{
		try
		{
			while(true)
			{
				Socket client = server.accept();
				System.out.println(client.getInetAddress());
				clients.add(new User(client));
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
				System.out.println(clients.size());
				for (User client : clients)
				{
					OutputStream out = client.getSocket().getOutputStream();
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
