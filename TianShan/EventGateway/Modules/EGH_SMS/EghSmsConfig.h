#ifndef __EGHSMSCONFIG_H__
#define __EGHSMSCONFIG_H__

#include <ConfigHelper.h>
#include <string>
#include <vector>

using namespace ZQ::common;
namespace EGH_SMS{

struct smsSubscriber
{
	std::string strTel;

	static void structure(Config::Holder< smsSubscriber > &holder)
	{
		holder.addDetail("", "phoneNumber", &smsSubscriber::strTel, NULL);
	}
};

struct smsEvent
{
	std::string strCategory;
	std::string strEventId;
	std::string strMessage;

	static void structure(Config::Holder< smsEvent > &holder)
    {
		holder.addDetail("", "category", &smsEvent::strCategory, NULL);
        holder.addDetail("", "eventId", &smsEvent::strEventId, NULL);
        holder.addDetail("", "message", &smsEvent::strMessage, NULL);
    }
};

struct smsEventGroup
{
	//subscribers
	typedef std::vector< Config::Holder< smsSubscriber > > SMSSUBSCRIBERS;
	SMSSUBSCRIBERS	subscribers;

	//events
	typedef std::vector< Config::Holder< smsEvent > > SMSEVENTS;
	SMSEVENTS events;

	static void structure(Config::Holder< smsEventGroup > &holder)
    {
		holder.addDetail("Subscriber", &smsEventGroup::readSubscriberProp, &smsEventGroup::registerSubscriberProp);

		holder.addDetail("Event", &smsEventGroup::readEventProp, &smsEventGroup::registerEventProp);
	}
	
	void readSubscriberProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<smsSubscriber> smsSubscriberholder("");
        smsSubscriberholder.read(node, hPP);
        subscribers.push_back(smsSubscriberholder);
	}

	void registerSubscriberProp(const std::string &full_path)
    {
        for (SMSSUBSCRIBERS::iterator it = subscribers.begin(); it != subscribers.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
	
	void readEventProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<smsEvent> smsEventholder("");
        smsEventholder.read(node, hPP);
        events.push_back(smsEventholder);
	}

	void registerEventProp(const std::string &full_path)
    {
        for (SMSEVENTS::iterator it = events.begin(); it != events.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
	
};


struct eghSms
{
	//log item
	int32	logLevel;
	int32	logSize;

	//event group
	typedef std::vector< Config::Holder< smsEventGroup > > SMSEVENTGROUP;
	SMSEVENTGROUP	eventgroups;

	//basic item
	std::string		portName;
	std::string		smsCN;
	int32			msgLLT;
	int32			bWideChar;
	int32			interval;

	static void structure(Config::Holder< eghSms > &holder)
    {
		holder.addDetail("EGH_SMS/Log", "level", &eghSms::logLevel, NULL,Config::optReadOnly);
		holder.addDetail("EGH_SMS/Log", "size", &eghSms::logSize, NULL,Config::optReadOnly);

		
        holder.addDetail("EGH_SMS/EventGroups/EventGroup", &eghSms::readEventGroupProp, &eghSms::registerEventGroupProp);
			
		holder.addDetail("EGH_SMS/MsgBasic", "portName", &eghSms::portName, NULL,Config::optReadOnly);
		holder.addDetail("EGH_SMS/MsgBasic", "smsCN", &eghSms::smsCN, NULL,Config::optReadOnly);
		holder.addDetail("EGH_SMS/MsgBasic", "msgLLT", &eghSms::msgLLT, NULL,Config::optReadOnly);
		holder.addDetail("EGH_SMS/MsgBasic", "bWideChar", &eghSms::bWideChar, NULL,Config::optReadOnly);
		holder.addDetail("EGH_SMS/MsgBasic", "sendInterval", &eghSms::interval, NULL,Config::optReadOnly);
		
	}


	void readEventGroupProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<smsEventGroup> smsEventGroupholder("");
        smsEventGroupholder.read(node, hPP);
        eventgroups.push_back(smsEventGroupholder);
	}

	void registerEventGroupProp(const std::string &full_path)
    {
        for (SMSEVENTGROUP::iterator it = eventgroups.begin(); it != eventgroups.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }

};












}

extern ZQ::common::Config::Loader <EGH_SMS::eghSms> gConfig;

#endif//__EGHSMSCONFIG_H__

