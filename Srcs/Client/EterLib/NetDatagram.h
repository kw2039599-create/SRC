#pragma once

class CNetworkDatagram
{
	public:
		CNetworkDatagram();
		virtual ~CNetworkDatagram();

		void Destroy();
		bool Create(UINT uPort);

		void Update();

		bool CanRecv();

		int PeekRecvFrom(UINT uBufLen, void* pvBuf, SOCKADDR_IN* pkSockAddrIn);
		int RecvFrom(UINT uBufLen, void* pvBuf, SOCKADDR_IN* pkSockAddrIn);
		int SendTo(UINT uBufLen, const void* c_pvBuf, const SOCKADDR_IN& c_rkSockAddrIn);

	private:
		void __Initialize();

	private:
		SOCKET m_sock;

		fd_set m_fdsRecv;
		fd_set m_fdsSend;
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
