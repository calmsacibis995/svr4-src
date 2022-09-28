
/*=================================================================*/
/*
**	The ident string is enclosed in a comment because
**	'rpcgen' will choke on it if it isn't.
**
#ident	"@(#)lp:lib/lpNet/xdrMsgs.x	1.2"
*/
%#define	MSGS_VERSION_MAJOR	1
%#define	MSGS_VERSION_MINOR	1

/*---------------------------------------------------------*/
/*
**  This is the logical message TAG of the
**  network message.
*/
enum	networkMsgType
{
	JobControlMsg	= 0,
	SystemIdMsg	= 1,
	DataPacketMsg	= 2,
	FileFragmentMsg	= 3,
	PacketBundleMsg	= 4
};
enum	jobControlCode
{
	NormalJobMsg	 = 0,
	RequestToSendJob = 1,
	ClearToSendJob	 = 2,
	AbortJob	 = 3,
	JobAborted	 = 4,
	RequestDenied	 = 5
};
struct	routingControl
{
	unsigned int	sysId;		/*  Id of the sending system.  */
	unsigned int	msgId;		/*  Id of the message (DG)     */
};
struct	jobControl
{
	unsigned char	controlCode;
	unsigned char	priority;
	unsigned char	endOfJob;
	unsigned int	jobId;
	         long	timeStamp;
};
struct	networkMsgTag
{
	unsigned char	versionMajor;	/*  Major and minor version    */
	unsigned char	versionMinor;	/*  number our message set.    */
	struct
	routingControl	routeControl;
	networkMsgType	msgType;
	struct
	jobControl	*jobControlp;
};

/*---------------------------------------------------------*/
/*
**  The message set.
*/
struct	systemIdMsg
{
	string	systemNamep <>;
	opaque	data <>;
};

struct	dataPacketMsg
{
	int	endOfPacket;
	opaque	data <>;
};

struct	packetBundleMsg
{
	struct
	dataPacketMsg	packets <>;
};

struct	fileFragmentMsg
{
	int		endOfFile;
	long		sizeOfFile;
	string		destPathp <>;
	opaque		fragment <>;
};
/*=================================================================*/
