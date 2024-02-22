# Tuple Space implementation

## About
Repository containing semestral project for "Programming Internet of Things Systems" course.
Project was about implementing tuple space as a server operating in Linux environment. Additionaly two separate Arduino nodes were created that were using tuple space to perform specific tasks. Arduino nodes were emulated using emulator provided by administrator of the course.

## Dependency Diagram 

![Dependency_Diagram](Dependency_diagram.png)


## Application Layer Protocol
         2                  14   15    16
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | PT |  sequenuce_number | OP | Ack |
    +----+-------------------+----+-----+
    |              Payload ...              
    +-----------------------------------+

## Tuple Space

## Tuple Space API
 
## App1

## App2

## Potential future improvments
* One of weaknesses of this implementation is its lack of proper serialization. Serializing data to MessagePack or other binary serialization method should be implemented.
* Server is really vurnelable in terms of interpreting received data. When bad data is delivered server interprets it as it was properly structured tuple. Data corectness checking should be added.
* ALP header's sequence number is not implemented. Potential future upgrade.
