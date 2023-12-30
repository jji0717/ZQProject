# -*- encoding:utf-8 -*-

import AquaClient
import json
from StringIO import StringIO

class PyAquaClient(AquaClient.NestedClient):
	def __init__(self,config, rooturl, userDomain, homecontainer):
		AquaClient.NestedClient.__init__(self, config, rooturl, userDomain, homecontainer)

	def cdmi( self,cmd, uri, arg ):
		(e,j) =self.doCdmi( cmd,uri,json.dumps(arg) )
		return (e,json.loads(j))

	def noncdmi( self,cmd, uri, arg, buf ):
		(e,j) = self.doNonCdmi( cmd, uri, json.dumps(arg),buf )
		return (e, json.loads(j))

	def cdmi_createDataObject( self, uri, mimetype="", metadata = {} ,
			value = "", valuetransferencoding = [] , domainURI = "", deserialize = "",
			serialize = "", copy = "", move="", reference="", deserializevalue="" ):
		arg = { "mimetype":mimetype,
				"metadata":metadata,
				"value":value,
				"valuetransferencoding":valuetransferencoding,
				"domainURI":domainURI,
				"deserialize":deserialize,
				"serialize":serialize,
				"copy":copy,
				"move":move,
				"reference":reference,
				"deserializevalue":deserializevalue}
		(e,jr) = self.cdmi("createDataObject",uri,arg)
		return (e, jr["result"])

	def cdmi_readDataObject( self, uri ):
		arg = {}
		(e,jr) = self.cdmi("readDataObject",uri, arg )
		#return error code AND result json AND location
		return (e, jr["result"], jr["location"])

	def cdmi_updateDataObjectEx( self, uri, metadata , startOffset, value, base_version=-1,
			partial=False, valuetransferencoding=[], domainURI="", deserialize="", 
			copy = "", deserializevalue = "" ):
		arg = {"metadata":metadata,
				"startOffset":startOffset,
				"value":value,
				"base_version":base_version,
				"partial":partial,
				"valuetransferencoding":valuetransferencoding,
				"domainURI":domainURI,
				"deserialize":deserialize,
				"copy":copy,
				"deserializevalue":deserializevalue	}
		(e,jr) = self.cdmi("updateDataObjectEx", uri, arg)
		return (e,jr["location"])

	def cdmi_updateDataObject( self, uri, metadata, startOffset, value="", partial=False, 
			valuetransferencoding = [] , domainURI="", deserialize="", copy="", deserializevalue="" ):
		arg = {
				"metadata":metadata,
				"startOffset":startOffset,
				"value":value,
				"partial":partial,
				"valuetransferencoding":valuetransferencoding,
				"domainURI":domainURI,
				"deserialize":deserialize,
				"copy":copy,
				"deserializevalue":deserializevalue	}
		(e,jr) = self.cdmi("updateDataObject",uri,arg)
		return (e,jr["location"])

	def cdmi_deleteDataObject( self, uri ):
		(e,jr) = self.cdmi("deleteDataObject",uri,{})
		return (e,jr)

	def cdmi_createContainer( self, uri, metadata, exports = {}, domainURI = "",
			deserialize = "", copy = "", move = "", reference="", deserializevalue="" ):
		arg = {"metadata":metadata,
				"exports":exports,
				"domainURI":domainURI,
				"deserialize":deserialize,
				"copy":copy,
				"move":move,
				"reference":reference,
				"deserializevalue":deserializevalue	}
		(e,jr) = self.cdmi("createContainer",uri, arg)
		return (e, jr["result"])

	def cdmi_readContainer( self, uri ):
		(e,jr) = self.cdmi("readContainer",uri,{})
		return (e,jr["result"])

	def cdmi_updateContainer( self, uri, metadata, exports={}, domainURI="", deserialize="",
			copy="", snapshot="", deserializevalue=""):
		arg= {"metadata":metadata,
				"exports":exports,
				"domainURI":domainURI,
				"deserialize":deserialize,
				"copy":copy,
				"snapshot":snapshot,
				"deserializevalue":deserializevalue	}
		(e,jr) = self.cdmi("updateContainer", uri, arg )
		return (e, jr["result"], jr["location"])

	def cdmi_deleteContainer( self, uri ):
		(e,jr) = self.cdmi("deleteContainer",uri,{})
		return (e,jr["result"])

	def cdmi_readDomain( self, uri):
		(e,jr) = self.cdmi("readDomain", uri, {} )
		return (e, jr["result"])

	def noncdmi_createDataObject( self, uri, contentType, value):
		arg = {"contentType":contentType,
				"value":value	}
		buf = ""
		(e,jr) = self.noncdmi("createDataObject", uri, arg, buf)
		return (e,jr["contentType"])

	def noncdmi_readDataObject( self, uri, startOffset, size = 4 * 1024 , disableCache = False):
		buf = bytearray(size)
		arg = {"startOffset":startOffset,
				"disableCache":disableCache}
		(e,jr) = self.noncdmi( "readDataObject",uri, arg,buf)
		size= jr['size']
		buf = buffer(buf,0,size)
		return (e,buf,size,jr["contentType"],jr["location"])

	def noncdmi_updateDataObject( self, uri, startOffset, buf, contentType = "", objectSize = -1, partial = False, disableCache = False):
		arg = {"startOffset":startOffset,
				"contentType":contentType,
				"objectSize":objectSize,
				"partial":partial,
				"disableCache":disableCache	}
		(e,jr) = self.noncdmi("updateDataObject", uri, arg, buf)
		return (e,jr["location"])

	def noncdmi_deleteDataObject( self, uri ):
		buf = ""
		arg= {}
		(e,jr) = self.noncdmi("deleteDataObject", uri, arg, buf )
		return e

	def noncdmi_createContainer( self, uri ):
		buf = ""
		arg = {}
		(e,jr) = self.noncdmi("createContainer", uri, arg,buf)
		return e

	def noncdmi_deleteContainer( self, uri ):
		buf = ""
		arg =  {}
		(e,jr) = self.noncdmi("deleteContainer", uri, arg , buf )
		return e
	def noncdmi_flushDataObject( self,uri):
		buf = ""
		arg = {}
		(e,jr) = self.noncdmi("flushDatObject", uri, arg, buf )
		return e

def test(conf):
	c = PyAquaClient(conf,"http://root:storage@172.16.20.40:8080/aqua/rest/cdmi","hongquan/test")
	print c.cdmi_readContainer("/")

if __name__ == "__main__":
	conf = None
	with open("client.config.json","rb") as f:
		conf = json.load(f)
	test( json.dumps(conf))
