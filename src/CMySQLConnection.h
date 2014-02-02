#pragma once
#ifndef INC_CMYSQLCONNECTION_H
#define INC_CMYSQLCONNECTION_H


#include <string>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <queue>

using std::string;
using boost::atomic;
using boost::thread;
using boost::function;

#ifdef WIN32
	#include <WinSock2.h>
#endif
#include <mysql/mysql.h>


class CMySQLQuery;


class CMySQLConnection
{
public:
	static CMySQLConnection *Create(string &host, string &user, string &passwd, string &db, size_t port, bool auto_reconnect, bool unthreaded = false);
	void Destroy();

	//(dis)connect to the MySQL server
	void Connect();
	void Disconnect();

	//escape a string to dest
	void EscapeString(const char *src, string &dest);

	inline MYSQL *GetMysqlPtr()
	{
		return m_Connection;
	}

	inline void QueueQuery(CMySQLQuery *query)
	{
		m_QueryQueue.push(query);
	}

	inline bool operator==(CMySQLConnection &rhs)
	{
		return (rhs.m_Host.compare(m_Host) == 0 && rhs.m_User.compare(m_User) == 0 && rhs.m_Database.compare(m_Database) == 0 && rhs.m_Passw.compare(m_Passw) == 0);
	}

private: //functions
	void ProcessQueries();
	inline void CloseThread()
	{
		m_QueryThreadRunning = false;
		m_QueryThread.join();
	}

private: //variables
	CMySQLConnection(string &host, string &user, string &passw, string &db, size_t port, bool auto_reconnect, bool unthreaded);
	~CMySQLConnection();


	thread m_QueryThread;
	atomic<bool> m_QueryThreadRunning;

	atomic<bool> m_IsUnthreaded;

	boost::lockfree::spsc_queue<
		CMySQLQuery *,
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<16876>
	> m_QueryQueue;

	boost::mutex m_FuncQueueMtx;
	std::queue<function<void()> > m_FuncQueue;


	//MySQL server login values
	string
		m_Host,
		m_User,
		m_Passw,
		m_Database;
	size_t m_Port;

	//connection status
	bool m_IsConnected;

	//automatic reconnect
	bool m_AutoReconnect;

	//internal MYSQL pointer
	MYSQL *m_Connection;
};


#endif // INC_CMYSQLCONNECTION_H
