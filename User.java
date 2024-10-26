import java.util.*;
import java.io.*;
import java.net.*;
import java.nio.charset.*;
import com.google.gson.*;
import com.google.gson.stream.*;

public class User
{
	private Socket _userSocket;
	private boolean _isPlay = false;
	private String _username = "Without name";
	private int _ID = 0;

	private OutputStream out;
	private InputStream in;


	public User(Socket sock) throws IOException
	{
		_userSocket = sock;
		out = sock.getOutputStream();
		in  = sock.getInputStream();
	}

	public String getUserAddress()
	{
		return _userSocket.getInetAddress().toString();
	}

	public int getUserPort()
	{
		return _userSocket.getPort();
	}

	public Socket getSocket()
	{
		return _userSocket;
	}

	public void setID(int id)
	{
		_ID = id;
	}

	public int getID()
	{
		return _ID;
	}

	public void set_name(String name)
	{
		_username = name;
	}

	public String get_name()
	{
		return _username;
	}

	public String receive_data() throws IOException
	{
		byte [] str = new byte[1024];
		int size = in.read(str);
		System.out.println("Name : " + _username + " " + size);
		return new String(str, StandardCharsets.UTF_8).substring(0, size - 1);
	}

	public void send_data(String str) throws IOException
	{
		byte [] respBytes = str.getBytes(StandardCharsets.US_ASCII);
		byte [] fullResp = new byte[respBytes.length + 1];
		System.arraycopy(respBytes, 0, fullResp, 0, respBytes.length);
		fullResp[respBytes.length] = 0;
		out.write(fullResp);
		out.flush();	
	}
}
