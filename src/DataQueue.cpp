#include "depthai/DataQueue.hpp"

#include "datatype/StreamPacketParser.hpp"
#include "depthai-shared/xlink/XLinkConstants.hpp"

namespace dai
{
    

DataOutputQueue::DataOutputQueue(std::shared_ptr<XLinkConnection> conn, std::string streamName) : connection(conn) {

    // creates a thread which reads from connection into the queue
    readingThread = std::thread([this, streamName](){

        try {
            
            // open stream with 1B write size (no writing will happen here)
            connection->openStream(streamName, 1);

            while(running){
                // read packet
                auto packet = connection->readFromStreamRaw(streamName);
                // parse packet
                auto data = parsePacket(packet);
                // release packet
                connection->readFromStreamRawRelease(streamName);
                // Add 'data' to queue
                queue.push(data);       
            }

            connection->closeStream(streamName);

        } catch (const std::exception& ex){
            // TODO
            assert(0 && "TODO");
        }
       

    });

}


DataOutputQueue::~DataOutputQueue(){
    // detach from thread, because currently no way to unblock underlying XLinkReadData
    running = false;
    readingThread.detach();
}



DataInputQueue::DataInputQueue(std::shared_ptr<XLinkConnection> conn, std::string streamName) : connection(conn) {

    // creates a thread which reads from connection into the queue
    writingThread = std::thread([this, streamName](){

        try {
            
            // open stream with 1B write size (no writing will happen here)
            connection->openStream(streamName, XLINK_USB_BUFFER_MAX_SIZE);

            while(running){
                // get data from queue
                std::shared_ptr<RawBuffer> data;
                queue.waitAndPop(data);

                // serialize 
                auto serialized = serializeData(data);

                // Write packet to device
                connection->writeToStream(streamName, serialized);
            }

            connection->closeStream(streamName);

        } catch (const std::exception& ex){
            // TODO
            assert(0 && "TODO");
        }
       

    });
}

DataInputQueue::~DataInputQueue(){
    // detach from thread, because currently no way to unblock underlying XLinkWriteData
    running = false;
    writingThread.detach();
}


void DataInputQueue::send(std::shared_ptr<RawBuffer> val){
    queue.push(val);
    queue.waitEmpty();
}

void DataInputQueue::sendAsync(std::shared_ptr<RawBuffer> val){
    queue.push(val);
}







} // namespace dai