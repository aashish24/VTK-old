/*=========================================================================
  
  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  
Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkSocketCommunicator.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"

#ifdef _WIN32
#define WSA_VERSION MAKEWORD(1,1)
#define vtkCloseSocketMacro(sock) (closesocket(sock))
#else
#define vtkCloseSocketMacro(sock) (close(sock))
#endif

const int vtkSocketCommunicator::MAX_MSG_SIZE=16000;

//------------------------------------------------------------------------------
vtkSocketCommunicator* vtkSocketCommunicator::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSocketCommunicator");
  if(ret)
    {
    return (vtkSocketCommunicator*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSocketCommunicator;
}

//----------------------------------------------------------------------------
vtkSocketCommunicator::vtkSocketCommunicator()
{
  this->Socket = -1;
  this->IsConnected = 0;
  this->NumberOfProcesses = 2;
}

//----------------------------------------------------------------------------
vtkSocketCommunicator::~vtkSocketCommunicator()
{
  if (this->IsConnected)
    {
    vtkCloseSocketMacro(this->Socket);
    }
}


//----------------------------------------------------------------------------
void vtkSocketCommunicator::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkCommunicator::PrintSelf(os,indent);
}

static inline int checkForError(int id, int maxId)
{
  if ( id == 0 )
    {
    vtkGenericWarningMacro("Can not connect to myself!");
    return 1;
    }
  else if ( id >= maxId )
    {
    vtkGenericWarningMacro("No port for process " << id << " exists.");
    return 1;
    }
  return 0;
}

template <class T>
static int sendMessage(T* data, int length, int tag, int sock)
{
  // Need to check the return value of these
  send(sock, (char *)&tag, sizeof(int), 0);

  int totalLength = length*sizeof(T);
  if ( totalLength < vtkSocketCommunicator::MAX_MSG_SIZE )
    send(sock, (char *)data, totalLength, 0);
  else
    {
    int num = totalLength/vtkSocketCommunicator::MAX_MSG_SIZE;
    for(int i=0; i<num; i++)
      {
      send(sock, &(((char *)data)[i*vtkSocketCommunicator::MAX_MSG_SIZE]), 
	   vtkSocketCommunicator::MAX_MSG_SIZE, 0);
      }
    send(sock, &(((char *)data)[num*vtkSocketCommunicator::MAX_MSG_SIZE]), 
	 totalLength-num*vtkSocketCommunicator::MAX_MSG_SIZE, 0);
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkSocketCommunicator::Send(int *data, int length, int remoteProcessId, 
				int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  return sendMessage(data, length, tag, this->Socket);
}

//----------------------------------------------------------------------------
int vtkSocketCommunicator::Send(unsigned long *data, int length, 
				int remoteProcessId, int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  return sendMessage(data, length, tag, this->Socket);
}
//----------------------------------------------------------------------------
int vtkSocketCommunicator::Send(char *data, int length, 
				int remoteProcessId, int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  return sendMessage(data, length, tag, this->Socket);
}
//----------------------------------------------------------------------------
int vtkSocketCommunicator::Send(float *data, int length, 
				int remoteProcessId, int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  return sendMessage(data, length, tag, this->Socket);
}

template <class T>
static int receiveMessage(T* data, int length, int tag, int sock)
{
  int recvTag=-1;

  // Need to check the return value of these
  recv(sock, (char *)&recvTag, sizeof(int), MSG_PEEK);
  if (recvTag != tag)
    return 0;

  recv(sock, (char *)&recvTag, sizeof(int), 0);
  int totalLength = length*sizeof(T);
  if ( totalLength < vtkSocketCommunicator::MAX_MSG_SIZE )
    recv(sock, (char *)data, totalLength, 0);
  else
    {
    int num = totalLength/vtkSocketCommunicator::MAX_MSG_SIZE;
    for(int i=0; i<num; i++)
      {
      recv(sock, &(((char *)data)[i*vtkSocketCommunicator::MAX_MSG_SIZE]), 
	   vtkSocketCommunicator::MAX_MSG_SIZE, 0);
      }
    recv(sock, &(((char *)data)[num*vtkSocketCommunicator::MAX_MSG_SIZE]), 
	 totalLength-num*vtkSocketCommunicator::MAX_MSG_SIZE, 0);
    }
  

  return 1;

}

//----------------------------------------------------------------------------
int vtkSocketCommunicator::Receive(int *data, int length, int remoteProcessId, 
				   int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  int id =  receiveMessage(data, length, tag, this->Socket);
  
  if ( tag == vtkMultiProcessController::RMI_TAG )
    {
    data[2] = id;
    }

  return id;
}

//----------------------------------------------------------------------------
int vtkSocketCommunicator::Receive(unsigned long *data, int length, 
				   int remoteProcessId, int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  return receiveMessage(data, length, tag, this->Socket);
}

//----------------------------------------------------------------------------
int vtkSocketCommunicator::Receive(char *data, int length, 
				   int remoteProcessId, int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  return receiveMessage(data, length, tag, this->Socket);
}

int vtkSocketCommunicator::Receive(float *data, int length, 
				   int remoteProcessId, int tag)
{
  if ( checkForError(remoteProcessId, this->NumberOfProcesses) )
    {
    return 0;
    }

  return receiveMessage(data, length, tag, this->Socket);
}


int vtkSocketCommunicator::WaitForConnection(int port, int timeout)
{

  if ( this->IsConnected )
    {
    vtkErrorMacro("Port " << 1 << " is occupied.");
    return 0;
    }

  int sock = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server;

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  if ( bind(sock, (sockaddr *)&server, sizeof(server)) )
    {
    vtkErrorMacro("Can not bind socket to port " << port);
    return 0;
    }
  listen(sock,1);
  this->Socket = accept(sock, 0, 0);
  if ( this->Socket == -1 )
    {
    vtkErrorMacro("Error in accept.");
    return 0;
    }
  vtkCloseSocketMacro(sock);
    
  this->IsConnected = 1;
  return 1;
}

void vtkSocketCommunicator::CloseConnection()
{
  if ( this->IsConnected )
    {
    vtkCloseSocketMacro(this->Socket);
    this->IsConnected = 0;
    }
}

int vtkSocketCommunicator::ConnectTo ( char* hostName, int port )
{

  if ( this->IsConnected )
    {
    vtkErrorMacro("Communicator port " << 1 << " is occupied.");
    return 0;
    }

  struct hostent* hp;
  hp = gethostbyname(hostName);
  if (!hp)
    {
    unsigned long addr = inet_addr(hostName);
    hp = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    }
  if (!hp)
    {
    vtkErrorMacro("Unknown host: " << hostName);
    return 0;
    }

  this->Socket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in name;
  name.sin_family = AF_INET;
  memcpy(&name.sin_addr, hp->h_addr, hp->h_length);
  name.sin_port = htons(port);

  if( connect(this->Socket, (sockaddr *)&name, sizeof(name)) < 0)
    {
    vtkErrorMacro("Can not connect to " << hostName << " on port " << port);
    return 0;
    }

  vtkDebugMacro("Connected to " << hostName << " on port " << port);
  this->IsConnected = 1;
  return 1;

}




