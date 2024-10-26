import java.util.*;
import java.util.concurrent.locks.*;
import java.io.*;
import java.net.*;
import java.nio.charset.*;
import com.google.gson.*;
import com.google.gson.stream.*;

public class Main
{
	public static Lock lock = new ReentrantLock();
	public static int count = 0;
	public static int roomsCount = 0;
	public static ServerSocket server;
	public static int port = 18881;
	public static List<User> clients = new ArrayList<User>();
	public static Map<String, Room> rooms = new HashMap<String, Room>();

	private static void waitConnection()
	{
		try
		{
			while(true)
			{
				Socket client = server.accept();
				System.out.println(client.getInetAddress());
				User user = new User(client);
				user.setID(count);
				lock.lock();
				clients.add(user);
				lock.unlock();
				count++;
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
				System.out.print("");
				lock.lock();
				Iterator<User> iter = clients.iterator();
				while (iter.hasNext())
				{
					User client = iter.next();
					System.out.println(client.get_name());
					
					String message = client.receive_data();
					System.out.println(message);
					if (message.length() > 0)
					{
						System.out.println("Message : " + message);
						JsonReader reader = new JsonReader(new StringReader(message));
						reader.setLenient(true);
						JsonObject jsonObj = JsonParser.parseReader(reader).getAsJsonObject();
						String type = jsonObj.get("type").getAsString();
						System.out.println(type);

						if (type.equals("create-room"))
						{
							Room room = new Room(2);
							String username = jsonObj.get("username").getAsString();
							client.set_name(username);
							room.add_user(client);
							iter.remove();
							rooms.put("" + roomsCount, room);
							String resp = "{\"type\":\"successful\", \"id\": " + roomsCount + " }";
							roomsCount++;
							client.send_data(resp);
						}
						if (type.equals("connect-to-room"))
						{
							String roomID = jsonObj.get("room-id").getAsString();
							String username = jsonObj.get("username").getAsString();
							Room room = rooms.get(roomID);
							if (room != null)
							{
								client.set_name(username);
								room.add_user(client);
								iter.remove();
								String resp = "{\"type\" : \"successful\"}";
								client.send_data(resp);
							}
							else 
								System.out.println("Cannot find room : " + roomID + " !");
						}
					}
					
					/*out.write(asciiTO);
					out.flush();
					*/

					
				}
				lock.unlock();

				Set<String> keys = rooms.keySet();
				for (String key : keys)
				{
					Room room = rooms.get(key);
					room.handle();
				}
			}
		}
		catch (IOException e)
		{
			System.out.println(e.getMessage());
		}
	}
}
