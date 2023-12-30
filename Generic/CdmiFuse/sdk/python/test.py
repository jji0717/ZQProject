#!/usr/bin/env python
#-*- encoding:utf-8 -*-

from aquaclient import PyAquaClient
import unittest
import json
import random
import hashlib

rooturl = "http://root:storage@172.16.20.131:8080/aqua/rest/cdmi"
homecontainer = "hongquan/test"

class FunctionalTest( unittest.TestCase ):
	def setUp(self):
		conf = None
		with open("client.config.json","rb") as f:
			conf = json.load(f)
		self._client = PyAquaClient( json.dumps(conf), rooturl, homecontainer )

	def teardown(self):
		self._client = None
	
	def genfile(self,name):
		candidates = "abcdefghijklmnopqrstuvwxyz"
		line = ""
		for i in range(0,16):
			line = line + random.choice(candidates) 
		line = line * 100 + "\n"
		count = random.randint(5000,5500)
		with open(name,"w+") as f:
			for i in range(0,count):
				f.write(line)

	def noncdmi_upload(self,name):
		offset = 0
		with open(name,"rb") as f:
			while(True):
				buf = f.read(64*1024)
				(e,loc) = self._client.noncdmi_updateDataObject(name,offset,buf)
				self.assertTrue(e>=200 and e<300)
				offset = offset + len(buf)
				if len(buf) != 64 * 1024:
					break

	def noncdmi_download(self,name,localname):
		offset = 0
		with open(localname,"w+b") as f:
			while True:
				(e,buf,size,_,_) = self._client.noncdmi_readDataObject(name,offset,64*1024)
				self.assertTrue(e>=200 and e<300)
				f.write(buf)
				offset = offset + size
				if( size != 64 * 1024):
					break
	
	def test_copyfile(self):
		'''
		Copy file from local disk to aqua server and 
		download file from aqua server to local disk
		'''
		name = "whahah"
		try:
			self.cdmi_deletedataobject(name)
			self.genfile(name)
			self.noncdmi_upload(name)
			self.noncdmi_download(name,"1")
			m = hashlib.md5()
			m.update(open(name).read())
			d1 = m.digest()
			m = hashlib.md5()
			m.update(open("1").read())
			d2 = m.digest()
			self.assertTrue(d1 == d2)
		finally:
			import os
			try:
				os.unlink(name)
			except:
				pass
			try:
				os.unlink("1")
			except:
				pass

	def cdmi_createcontainer(self, name):
		(e,jr) = self._client.cdmi_createContainer( name, {} )
		self.assertTrue( (e >= 200 and e < 300 ) or e == 409 )

	def cdmi_readcontainer(self,name):
		(e,jr) = self._client.cdmi_readContainer( name )
		self.assertTrue( e >= 200 and e < 300 )
	def cdmi_deletecontainer(self,name):
		(e,jr) = self._client.cdmi_deleteContainer( name )
		self.assertTrue( e >= 200 and e < 300 )

	def test_cdmi_container(self):
		'''
		test cdmi fcuntion
		'''
		name = "test_folder"
		self.cdmi_createcontainer( name )
		self.cdmi_readcontainer( name )
		self.cdmi_deletecontainer( name )

	def cdmi_createdataobject(self,name):
		(e,jr) = self._client.cdmi_createDataObject(name)
		self.assertTrue( e>= 200 and e<300)
	
	def cdmi_updatedataobject(self,name,data):
		(e,jr) = self._client.cdmi_updateDataObject(name,{},0,data)
		self.assertTrue( e>=200 and e<300)
	
	def cdmi_readdataobject(self,name):
		(e,jr,_) = self._client.cdmi_readDataObject(name)
		self.assertTrue( e>=200 and e<300)

	def cdmi_deletedataobject(self,name):
		(e,jr) = self._client.cdmi_deleteDataObject(name)
		self.assertTrue(e==404 or( e>=200 and e<300) )

	def test_cdmi_dataobject(self):
		'''
		test non-cdmi function
		'''
		name="test_object"
		self.cdmi_createdataobject(name)
		self.cdmi_updatedataobject(name,"wahaha")
		self.cdmi_readdataobject(name)
		self.cdmi_deletedataobject(name)
	
	def noncdmi_createcontainer(self, name):
		e = self._client.noncdmi_createContainer(name)
		self.assertTrue( ( e>= 200 and e < 300 ) or e == 409 )
	
	def noncdmi_deletecontainer(self,name):
		e = self._client.noncdmi_deleteContainer(name)
		self.assertTrue( e>=200 and e<300)
	
	def test_noncdmi_container(self):
		name="noncdmi_testfolder"
		self.noncdmi_createcontainer(name)
		self.noncdmi_deletecontainer(name)

	def noncdmi_createdataobject(self,name):
		(e,ct) = self._client.noncdmi_createDataObject(name,"","wahaha")
		self.assertTrue( e>= 200 and e < 300 )
	def noncdmi_readdataobject(self,name):
		(e,buf,_,_,_) = self._client.noncdmi_readDataObject(name,0)
		self.assertTrue( e>=200 and e<300)
	def noncdmi_updatedataobject(self,name):
		(e,loc) = self._client.noncdmi_updateDataObject(name,0,"1236756743683724")
		self.assertTrue(e>=200 and e<300)
	def noncdmi_deletedataobject(self,name):
		e = self._client.noncdmi_deleteDataObject(name)
		self.assertTrue(e>=200 and e<300)
	
	def test_noncdmi_dataobject(self):
		name="noncdmi_file"
		self.noncdmi_createdataobject(name)
		self.noncdmi_readdataobject(name)
		self.noncdmi_updatedataobject(name)
		self.noncdmi_deletedataobject(name)

	def test_cdmi_readDomain(self):
		(e,jr) =  self._client.cdmi_readDomain("")
		self.assertTrue( e >= 200 and e < 300 )

if __name__ == "__main__":
	import os
	try:
		os.unlink('cdmiclient.log')
	except:
		pass
	unittest.main()

