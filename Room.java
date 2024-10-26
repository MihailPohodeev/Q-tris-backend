import java.util.*;
import java.io.*;
import java.net.*;
import java.nio.charset.*;
import com.google.gson.*;
import com.google.gson.stream.*;

public class Room
{
	private List<User> _users = new ArrayList<User>();
	private int _count = 2;

	public Room(int count)
	{
		_count = count;
	}

	public void add_user(User user)
	{
		_users.add(user);
	}

	public void handle()
	{
		if (_users.size() < _count)
		{
			return;
		}

		Iterator<User> iter = _users.iterator();
		while(iter.hasNext())
		{
			User user = iter.next();
			
			//user.get
		}
	}
}
