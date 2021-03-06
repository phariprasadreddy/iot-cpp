/*******************************************************************************
 * Copyright (c) 2016 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Mike Tran - initial API and implementation and/or initial documentation
 *    Hari Prasada Reddy - Added functionalities/documentation to standardize with other client libraries
 *******************************************************************************/

#ifndef IOTF_CLIENT_H_
#define IOTF_CLIENT_H_


#include <queue>
#include "mqtt/async_client.h"
#include "mqtt/exception.h"
#include "json/json.h"
#include "IOTP_MessageHandler.h"
#include "CommandCallback.h"
#include "Properties.h"
#include "IOTP_Device.h"
#include "IOTP_DeviceActionHandler.h"
#include "IOTP_DeviceFirmwareHandler.h"
#include "IOTP_DeviceAttributeHandler.h"
#include "IOTP_ResponseHandler.h"

namespace Watson_IOTP {

	extern const unsigned long DEFAULT_TIMEOUT();

	class IOTP_Client {
		private:
			/**
			 * Nested class IOTF_Callback is private.  It can only be used by IOTF_Client.
			 */
			class IOTF_Callback : public virtual mqtt::callback {
				public:
					/** Pointer type for this object */
					typedef std::shared_ptr<IOTF_Callback> ptr_t;

					IOTF_Callback(IOTP_Client* iotf_client);
					~IOTF_Callback();
					void connection_lost(const std::string& cause);
					void message_arrived(const std::string& topic, mqtt::message_ptr msg);
					void delivery_complete(mqtt::idelivery_token_ptr tok);
					void set_callback(CommandCallback* cb);

					/**
					 * Wait for the response to the request.
					 *
					 * @param timeout in miliseconds
					 * @param ID of the request
					 *
					 * @return pointer to MQTT message
					 */
					mqtt::message_ptr const wait_for_response(long timeout, const std::string& reqId);

					std::vector<std::string> get_subscriptions();
					void add_subscription(std::string topic);
					void add_subscription(std::string topic, iotp_message_handler_ptr handler);
					void remove_subscription(std::string topic);
					void remove_subscription(std::string topic, iotp_message_handler_ptr handler);
					bool check_subscription(std::string topic);
					bool check_subscription(std::string topic, iotp_message_handler_ptr handler);

				private:
					int get_arrived_messages();

					int mArrivedMessages;
					std::vector<std::string> mSubscriptions;
					std::map<std::string, mqtt::message_ptr> mMessages;
					std::multimap<std::string, iotp_message_handler_ptr> mHandlers;
					//mqtt::callback* user_callback;
					IOTP_Client* mClient;
					CommandCallback* user_callback;
			};

			typedef IOTF_Callback::ptr_t iotf_callback_ptr;

			/**
			 * Nested class IOTF_ActionCallback is private. It can only be used by IOTF_Client.
			 */
			class IOTF_ActionCallback : public virtual mqtt::iaction_listener {
				public:
					IOTF_ActionCallback();
					/**
					 * This method is invoked when an action fails.
					 * @param asyncActionToken
					 */
					virtual void on_failure(const mqtt::itoken& asyncActionToken);
					/**
					 * This method is invoked when an action has completed successfully.
					 * @param asyncActionToken
					 */
					virtual void on_success(const mqtt::itoken& asyncActionToken);
					bool success();
				private:
					bool mSuccess;
			};


		public:
			/** Pointer type for this object */
			typedef std::shared_ptr<IOTP_Client> ptr_t;

			/**
			 * Constructor of an IOTF_Client.  The parameters are the same as
			 * the constructor of the base class, mqtt::callback.
			 */
			//IOTP_Client(const std::string& uri, const std::string& clientId);
			IOTP_Client(Properties& prop);

			/**
			 * Constructor of an IOTF_Client.  The parameters are the same as
			 * the constructor of the base class, mqtt::callback.
			 */
			//IOTP_Client(const std::string& uri, const std::string& clientId);
			//IOTP_Client(Properties& prop, Watson_IOTP::IOTP_DeviceInfo* deviceInfo);
			IOTP_Client(Properties& prop, iotf_device_data_ptr& deviceData);

			/**
			 * Constructor of an IOTF_Client.
			 * Required arguments are organization ID, device type and device ID.
			 */
			//IOTP_Client(Properties& prop, Watson_IOTP::IOTP_DeviceInfo* deviceInfo,
			IOTP_Client(Properties& prop, iotp_device_action_handler_ptr& actionHandler,
					iotp_device_firmware_handler_ptr& firmwareHandler);

			virtual ~IOTP_Client();


			/**
			 * Function sets the "keep Alive" interval.
			 *
			 * @param keepAliveInterval
			 * @return void
			 *
			 */

			void setKeepAliveInterval(int keepAliveInterval);

			/**
			 * Connect to Watson IoT Platform messaging server using default options.
			 *
			 * @return bool
			 * returns true if connection succeeds else fasle
			 * @throw MQTT exceptions
			 */
			bool connect()
				throw(mqtt::exception, mqtt::security_exception) ;

			/**
			 * Connect to Watson IoT Platform messaging server using default options.
			 *
			 * @param cb Action listener callback
			 * @return bool
			 * returns true if connection succeeds else fasle
			 * @throw MQTT exceptions
			 */
			bool connect(mqtt::iaction_listener& cb)
				throw(mqtt::exception, mqtt::security_exception) ;

			/**
			 * Gives information whether a client is Connected to Watson IoT Platform.
			 *
			 * @return bool
			 */
			bool isConnected() { return pasync_client->is_connected(); }
			/**
			* Function used to set the Command Callback function. This must be set if you to receive commands.
			*
			* @param cb - A pointer to the commandCallback.
			* @return void
			*/
			void setCommandHandler(CommandCallback* cb) ;


			/**
			 * Function used to Publish messages on a specific topic to the IBM Watson IoT service
			 * @param topic - topic on which message is sent
			 * @param message - message to be posted
			 * @return mqtt::idelivery_token_ptr
			 */
			mqtt::idelivery_token_ptr publishTopic(std::string topic, mqtt::message_ptr message);
			mqtt::idelivery_token_ptr publishTopic(std::string topic, mqtt::message_ptr message,
										void* userContext, mqtt::iaction_listener& cb);

			bool subscribeTopic(const std::string& topic, int qos);

			/**
			 * Function used to subscribe handler for each topic from the IBM Watson IoT service
			 * @return bool
			 * returns true if topic subscribed successfully else false
			 */
			bool subscribeCommandHandler(const std::string& topic, iotp_message_handler_ptr handler);

			/**
			 * Function used to unsubscribe topic from the IBM Watson IoT service
			 * @return bool
			 * returns true if topic is unsubscribed successfully else false
			 */
			bool unsubscribeCommands(const std::string& topic);

			/**
			 * Function used to disconnect the device from the IBM Watson IoT service
			 * @return void
			 */
			void disconnect() throw(mqtt::exception) ;

			bool supportDeviceActions() const;
			bool supportFirmwareActions() const;

			void IOTF_send_reply(iotp_reply_message_ptr reply);
			static std::string jsonValueToString(Json::Value& jsonValue);

		protected:
			std::string send_message(const std::string& topic, const Json::Value& data, int qos = 1);
			bool pushManageMessage(std::string topic, Json::Value data);
			virtual void InitializeMqttClient() = 0;
			mqtt::async_client* pasync_client;
			iotp_response_handler_ptr mResponseHandler;
			iotp_device_action_handler_ptr mActionHandler;
			iotp_device_firmware_handler_ptr mFirmwareHandler;
			Properties mProperties;

		private:

			void _send_reply();
			iotf_callback_ptr set_callback();
			void InitializeProperties(Properties& prop);
			unsigned long mReqCounter;
			iotf_callback_ptr callback_ptr;
			mutable std::mutex mLock;
			mutable std::condition_variable mCond;
			std::queue<iotp_reply_message_ptr> mReplyMsgs;
			std::thread mReplyThread;
			bool mExit;
			int mKeepAliveInterval;
			//////////////////////////////////////////////////////////////////////


//			int mLifetime;
			//Watson_IOTP::IOTP_DeviceInfo* mDeviceInfo;

			//int mReqCounter;
			///////////////////////////////////////////////////////////////////////

	};

	typedef IOTP_Client::ptr_t iotp_client_ptr;
}


#endif /* IOTF_CLIENT_H_ */
