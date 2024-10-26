import java.util.*;
import java.net.*;

public class User
{
	private Socket _userSocket;
	private boolean _isPlay = false;

	public User(Socket sock)
	{
		_userSocket = sock;
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
}
