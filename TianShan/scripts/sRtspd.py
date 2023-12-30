"""\
// An RTSP server simulator that responses based on pre-defined template
// Author: Hui Shao / hui.shao@xor-media.tv
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/scripts/srtspd.py $
"""

import socket, select
import re

C2Server="192.168.22.33"

playlist_template='''
<Playlist expiration="2014-05-12T23:24:59Z" >
        <Item assetId="EGEZ0595967209325440" providerId="so.xor-media.com" cueIn="0.00" cueOut="0.00" signature="2D7DDBBBA05B8F3498077BFC216258AB" />
        <Item assetId="EGEZ0595967209325488" providerId="so.xor-media.com" cueIn="0.00" cueOut="0.00" signature="6D7DDBBBF05B8F559AA744FC216338AB" />
</Playlist>
'''
SERVER_DESC="TianShan/2.0; rtsp_template_server/2.0"
setup_resp_template = \
"RTSP/1.0 200 OK\r\n"\
"CSeq: ${cseq}\r\n" \
"Server: " + SERVER_DESC +"\r\n" \
"Require: com.comcast.ngod.s1\r\n" \
"Session: ${sess};timeout=1200\r\n" \
"Transport: ${transport};source=" + C2Server +";server_port=12000\r\n" \
"ClientSessionId: ${clientSessId}\r\n" \
"TianShan-ClientTimeout: 1200\r\n" \
"TianShan-Version: 2.0\r\n" \
"Content-Type: text/xml\r\n" \
"Content-Length: " + str(len(playlist_template)) +"\r\n\r\n" + playlist_template

default_resp_template = \
"RTSP/1.0 200 OK\r\n" \
"CSeq: ${cseq}\r\n" \
"Server: " + SERVER_DESC +"\r\n" \
"Require: com.comcast.ngod.s1\r\n" \
"Session: ${sess}\r\n\r\n"

lastSessId = 1000

def sRtspd_handle(req) :
    resp = default_resp_template;
    se = re.search('[\s]*([^\s]+)[\s]+([^\s]+)[\s]*RTSP/1\.0', req)
    if not se:
        return "RTSP/1.0 400 Bad Request\r\nServer: " + SERVER_DESC +"\r\n\r\n";
    
    verb = se.group(1)
    uri = se.group(2)
    (sess, cseq, clisess) = ("","","")
    se = re.search('[\s]*Session:[\s]*([^\s]+)', req)
    if se:
        sess = se.group(1);
    resp = resp.replace('${sess}', sess)
    
    se = re.search('[\s]*CSeq:[\s]*([^\s]+)', req)
    if se:
        cseq = se.group(1);
 
    se = re.search('[\s]*ClientSessionId:[\s]*([^\s]+)', req)
    if se:
        clisess = se.group(1);
         
    if 'SETUP' == verb :
        se = re.search('[\s]*Transport:[\s*](MP2T/AVP/C2;unicast;.*destination=([^;]+);[^\s]*)', req)
        if se:
            transport= se.group(1)
            client = se.group(2)
            
        global lastSessId
        lastSessId = lastSessId +1
        resp = setup_resp_template
        resp = resp.replace('${sess}', str(lastSessId))
        resp = resp.replace('${transport}', transport)
    elif 'TEARDOWN' == verb :
        pass
    else :
        pass

    resp = resp.replace('${cseq}', cseq)
    resp = resp.replace('${clientSessId}', clisess)
    
    return resp;

def simple_rtsp_server(port=0):
    solist = []    # list of socket clients
    recvBuf = 4096 # Advisable to keep it as an exponent of 2
    
    if port <= 0:
        port = 10554
         
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # this has no effect, why ?
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("0.0.0.0", port))
    server_socket.listen(10)
 
    # Add server socket to the list of readable connections
    solist.append(server_socket)
 
    print "RTSP server starts listening on port %d..." % port
    req = ""
 
    while 1:
        # Get the list sockets which are ready to be read through select
        read_sockets,write_sockets,error_sockets = select.select(solist,[],[])
 
        for sock in read_sockets:
            #New connection
            if sock == server_socket:
                # Handle the case in which there is a new connection recieved through server_socket
                sockfd, addr = server_socket.accept()
                solist.append(sockfd)
                print "client[%s, %s] connected" % addr
            #Some incoming message from a client
            else:
                # Data recieved from client, process it
                try:
                    data = sock.recv(recvBuf)
                    if data:
                        req = req + data
                        pos =0
                        while (pos>=0 and pos <2) :
                            pos = max(req.find('\n\r\n'), req.find('\n\n'))
                            if (pos>=0 and pos <2) :
                                req = req[pos+2:]
                                # print "trim req: "+req

                        if pos <0 :
                            # print "incomplete request:" +req
                            continue
                        else :
                            print "Request Received:\n" +req
                            resp = sRtspd_handle(req)
                            req =""
                            if resp :
                                sock.send(resp)
                                print "Response Sent:\n" +resp
                 
                # client disconnected, so remove from socket list
                except:
                    sock.close()
                    print "client[%s, %s] disconnected" % addr
                    solist.remove(sock)
                    continue
         
    server_socket.close()
    
if __name__ == "__main__":
    simple_rtsp_server()
