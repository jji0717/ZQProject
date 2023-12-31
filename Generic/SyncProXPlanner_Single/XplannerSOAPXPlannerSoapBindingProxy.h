/* XplannerSOAPXPlannerSoapBindingProxy.h
   Generated by gSOAP 2.7.10 from Xplanner.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef XplannerSOAPXPlannerSoapBindingProxy_H
#define XplannerSOAPXPlannerSoapBindingProxy_H
#include "XplannerSOAPH.h"
class XPlannerSoapBinding
{   public:
	/// Runtime engine context allocated in constructor
	struct soap *soap;
	/// Endpoint URL of service 'XPlannerSoapBinding' (change as needed)
	const char *endpoint;
	/// Constructor allocates soap engine context, sets default endpoint URL, and sets namespace mapping table
	XPlannerSoapBinding()
	{ soap = soap_new(); endpoint = "http://sp-server:9090/soap/XPlanner"; if (soap && !soap->namespaces) { static const struct Namespace namespaces[] = 
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns2", "http://xml.apache.org/xml-soap", NULL, NULL},
	{"ns3", "http://domain.soap.xplanner.technoetic.com", NULL, NULL},
	{"ns4", "http://xplanner.org/soap", NULL, NULL},
	{"ns5", "http://db.xplanner.technoetic.com", NULL, NULL},
	{"ns1", "http://sp-server:9090/soap/XPlanner", NULL, NULL},
	{"ns6", "http://soap.xplanner.technoetic.com", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	soap->namespaces = namespaces; } };
	/// Destructor frees deserialized data and soap engine context
	virtual ~XPlannerSoapBinding() { if (soap) { soap_destroy(soap); soap_end(soap); soap_free(soap); } };
	/// Invoke 'getAttributes' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getAttributes(int _objectId, struct ns6__getAttributesResponse &_param_1) { return soap ? soap_call_ns6__getAttributes(soap, endpoint, NULL, _objectId, _param_1) : SOAP_EOM; };
	/// Invoke 'update' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__update(ns4__NoteData *_note, struct ns6__updateResponse &_param_2) { return soap ? soap_call_ns6__update(soap, endpoint, NULL, _note, _param_2) : SOAP_EOM; };
	/// Invoke 'update' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__update_(ns4__NoteData *_note, struct ns6__updateResponse_ &_param_3) { return soap ? soap_call_ns6__update_(soap, endpoint, NULL, _note, _param_3) : SOAP_EOM; };
	/// Invoke '' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__update__(ns4__NoteData *_note, struct ns6__updateResponse__ &_param_4) { return soap ? soap_call_ns6__update__(soap, endpoint, NULL, _note, _param_4) : SOAP_EOM; };
	/// Invoke '' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__update___(ns4__NoteData *_note, struct ns6__updateResponse___ &_param_5) { return soap ? soap_call_ns6__update___(soap, endpoint, NULL, _note, _param_5) : SOAP_EOM; };
	/// Invoke '' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__update____(ns4__NoteData *_note, struct ns6__updateResponse____ &_param_6) { return soap ? soap_call_ns6__update____(soap, endpoint, NULL, _note, _param_6) : SOAP_EOM; };
	/// Invoke '' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__update_____(ns4__NoteData *_note, struct ns6__updateResponse_____ &_param_7) { return soap ? soap_call_ns6__update_____(soap, endpoint, NULL, _note, _param_7) : SOAP_EOM; };
	/// Invoke '' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__update______(ns4__NoteData *_note, struct ns6__updateResponse______ &_param_8) { return soap ? soap_call_ns6__update______(soap, endpoint, NULL, _note, _param_8) : SOAP_EOM; };
	/// Invoke 'getAttribute' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getAttribute(int _objectId, char *_key, char *&_getAttributeReturn) { return soap ? soap_call_ns6__getAttribute(soap, endpoint, NULL, _objectId, _key, _getAttributeReturn) : SOAP_EOM; };
	/// Invoke 'setAttribute' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__setAttribute(int _objectId, char *_key, char *_value, struct ns6__setAttributeResponse &_param_9) { return soap ? soap_call_ns6__setAttribute(soap, endpoint, NULL, _objectId, _key, _value, _param_9) : SOAP_EOM; };
	/// Invoke 'getNote' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getNote(int _id, struct ns6__getNoteResponse &_param_10) { return soap ? soap_call_ns6__getNote(soap, endpoint, NULL, _id, _param_10) : SOAP_EOM; };
	/// Invoke 'removeNote' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__removeNote(int _id, struct ns6__removeNoteResponse &_param_11) { return soap ? soap_call_ns6__removeNote(soap, endpoint, NULL, _id, _param_11) : SOAP_EOM; };
	/// Invoke 'getPerson' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getPerson(int _id, struct ns6__getPersonResponse &_param_12) { return soap ? soap_call_ns6__getPerson(soap, endpoint, NULL, _id, _param_12) : SOAP_EOM; };
	/// Invoke 'getIterations' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getIterations(int _projectId, struct ns6__getIterationsResponse &_param_13) { return soap ? soap_call_ns6__getIterations(soap, endpoint, NULL, _projectId, _param_13) : SOAP_EOM; };
	/// Invoke 'getCurrentIteration' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getCurrentIteration(int _projectId, struct ns6__getCurrentIterationResponse &_param_14) { return soap ? soap_call_ns6__getCurrentIteration(soap, endpoint, NULL, _projectId, _param_14) : SOAP_EOM; };
	/// Invoke 'getUserStories' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getUserStories(int _containerId, struct ns6__getUserStoriesResponse &_param_15) { return soap ? soap_call_ns6__getUserStories(soap, endpoint, NULL, _containerId, _param_15) : SOAP_EOM; };
	/// Invoke 'getTasks' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getTasks(int _containerId, struct ns6__getTasksResponse &_param_16) { return soap ? soap_call_ns6__getTasks(soap, endpoint, NULL, _containerId, _param_16) : SOAP_EOM; };
	/// Invoke 'getTimeEntries' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getTimeEntries(int _containerId, struct ns6__getTimeEntriesResponse &_param_17) { return soap ? soap_call_ns6__getTimeEntries(soap, endpoint, NULL, _containerId, _param_17) : SOAP_EOM; };
	/// Invoke 'getCurrentTasksForPerson' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getCurrentTasksForPerson(int _personId, struct ns6__getCurrentTasksForPersonResponse &_param_18) { return soap ? soap_call_ns6__getCurrentTasksForPerson(soap, endpoint, NULL, _personId, _param_18) : SOAP_EOM; };
	/// Invoke 'addTask' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__addTask(ns4__TaskData *_task, struct ns6__addTaskResponse &_param_19) { return soap ? soap_call_ns6__addTask(soap, endpoint, NULL, _task, _param_19) : SOAP_EOM; };
	/// Invoke 'getPeople' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getPeople(struct ns6__getPeopleResponse &_param_20) { return soap ? soap_call_ns6__getPeople(soap, endpoint, NULL, _param_20) : SOAP_EOM; };
	/// Invoke 'getProject' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getProject(int _id, struct ns6__getProjectResponse &_param_21) { return soap ? soap_call_ns6__getProject(soap, endpoint, NULL, _id, _param_21) : SOAP_EOM; };
	/// Invoke 'getIteration' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getIteration(int _id, struct ns6__getIterationResponse &_param_22) { return soap ? soap_call_ns6__getIteration(soap, endpoint, NULL, _id, _param_22) : SOAP_EOM; };
	/// Invoke 'getTask' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getTask(int _id, struct ns6__getTaskResponse &_param_23) { return soap ? soap_call_ns6__getTask(soap, endpoint, NULL, _id, _param_23) : SOAP_EOM; };
	/// Invoke 'getUserStory' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getUserStory(int _id, struct ns6__getUserStoryResponse &_param_24) { return soap ? soap_call_ns6__getUserStory(soap, endpoint, NULL, _id, _param_24) : SOAP_EOM; };
	/// Invoke 'getProjects' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getProjects(struct ns6__getProjectsResponse &_param_25) { return soap ? soap_call_ns6__getProjects(soap, endpoint, NULL, _param_25) : SOAP_EOM; };
	/// Invoke 'addProject' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__addProject(ns4__ProjectData *_project, struct ns6__addProjectResponse &_param_26) { return soap ? soap_call_ns6__addProject(soap, endpoint, NULL, _project, _param_26) : SOAP_EOM; };
	/// Invoke 'removeProject' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__removeProject(int _id, struct ns6__removeProjectResponse &_param_27) { return soap ? soap_call_ns6__removeProject(soap, endpoint, NULL, _id, _param_27) : SOAP_EOM; };
	/// Invoke 'addIteration' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__addIteration(ns4__IterationData *_iteration, struct ns6__addIterationResponse &_param_28) { return soap ? soap_call_ns6__addIteration(soap, endpoint, NULL, _iteration, _param_28) : SOAP_EOM; };
	/// Invoke 'removeIteration' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__removeIteration(int _id, struct ns6__removeIterationResponse &_param_29) { return soap ? soap_call_ns6__removeIteration(soap, endpoint, NULL, _id, _param_29) : SOAP_EOM; };
	/// Invoke 'addUserStory' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__addUserStory(ns4__UserStoryData *_story, struct ns6__addUserStoryResponse &_param_30) { return soap ? soap_call_ns6__addUserStory(soap, endpoint, NULL, _story, _param_30) : SOAP_EOM; };
	/// Invoke 'removeUserStory' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__removeUserStory(int _id, struct ns6__removeUserStoryResponse &_param_31) { return soap ? soap_call_ns6__removeUserStory(soap, endpoint, NULL, _id, _param_31) : SOAP_EOM; };
	/// Invoke 'getPlannedTasksForPerson' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getPlannedTasksForPerson(int _personId, struct ns6__getPlannedTasksForPersonResponse &_param_32) { return soap ? soap_call_ns6__getPlannedTasksForPerson(soap, endpoint, NULL, _personId, _param_32) : SOAP_EOM; };
	/// Invoke 'removeTask' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__removeTask(int _id, struct ns6__removeTaskResponse &_param_33) { return soap ? soap_call_ns6__removeTask(soap, endpoint, NULL, _id, _param_33) : SOAP_EOM; };
	/// Invoke 'getTimeEntry' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getTimeEntry(int _id, struct ns6__getTimeEntryResponse &_param_34) { return soap ? soap_call_ns6__getTimeEntry(soap, endpoint, NULL, _id, _param_34) : SOAP_EOM; };
	/// Invoke 'addTimeEntry' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__addTimeEntry(ns4__TimeEntryData *_timeEntry, struct ns6__addTimeEntryResponse &_param_35) { return soap ? soap_call_ns6__addTimeEntry(soap, endpoint, NULL, _timeEntry, _param_35) : SOAP_EOM; };
	/// Invoke 'removeTimeEntry' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__removeTimeEntry(int _id, struct ns6__removeTimeEntryResponse &_param_36) { return soap ? soap_call_ns6__removeTimeEntry(soap, endpoint, NULL, _id, _param_36) : SOAP_EOM; };
	/// Invoke 'addNote' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__addNote(ns4__NoteData *_note, struct ns6__addNoteResponse &_param_37) { return soap ? soap_call_ns6__addNote(soap, endpoint, NULL, _note, _param_37) : SOAP_EOM; };
	/// Invoke 'getNotesForObject' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getNotesForObject(int _attachedToId, struct ns6__getNotesForObjectResponse &_param_38) { return soap ? soap_call_ns6__getNotesForObject(soap, endpoint, NULL, _attachedToId, _param_38) : SOAP_EOM; };
	/// Invoke 'addPerson' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__addPerson(ns4__PersonData *_object, struct ns6__addPersonResponse &_param_39) { return soap ? soap_call_ns6__addPerson(soap, endpoint, NULL, _object, _param_39) : SOAP_EOM; };
	/// Invoke 'removePerson' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__removePerson(int _id, struct ns6__removePersonResponse &_param_40) { return soap ? soap_call_ns6__removePerson(soap, endpoint, NULL, _id, _param_40) : SOAP_EOM; };
	/// Invoke 'deleteAttribute' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__deleteAttribute(int _objectId, char *_key, struct ns6__deleteAttributeResponse &_param_41) { return soap ? soap_call_ns6__deleteAttribute(soap, endpoint, NULL, _objectId, _key, _param_41) : SOAP_EOM; };
	/// Invoke 'getAttributesWithPrefix' of service 'XPlannerSoapBinding' and return error code (or SOAP_OK)
	virtual int ns6__getAttributesWithPrefix(int _objectId, char *_prefix, struct ns6__getAttributesWithPrefixResponse &_param_42) { return soap ? soap_call_ns6__getAttributesWithPrefix(soap, endpoint, NULL, _objectId, _prefix, _param_42) : SOAP_EOM; };
};
#endif
