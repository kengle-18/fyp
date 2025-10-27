package com.example.grpcHelper

import io.grpc.{ManagedChannel, ManagedChannelBuilder, Server, ServerBuilder}
import scala.concurrent.{ExecutionContext, Future}
import java.util.concurrent.Executors
import org.slf4j.LoggerFactory

// Import the generated ScalaPB classes
import com.example.base._ // adjust based on the generated folder

// 1️⃣ Server Implementation
class UniversalTesterImpl extends UniversalTesterGrpc.UniversalTester {

  override def sendUniversal(request: UniversalMessage): Future[UniversalMessage] =
    Future.successful(request) // simply echo back for testing
}

// 2️⃣ gRPC Server
class GrpcServer(executionContext: ExecutionContext, port: Int) {

  val server: Server = ServerBuilder
    .forPort(port)
    .addService(UniversalTesterGrpc.bindService(new UniversalTesterImpl, executionContext))
    .build()

  def start(): Unit = {
    server.start()
    println(s"Server started, listening on ${server.getPort}")
  }

  def stop(): Unit = server.shutdown()
  def blockUntilShutdown(): Unit = server.awaitTermination()
}

// 3️⃣ gRPC Client
class GrpcClient(host: String, port: Int) {

  val channel: ManagedChannel = ManagedChannelBuilder
    .forAddress(host, port)
    .usePlaintext()
    .build()

  val blockingStub: UniversalTesterGrpc.UniversalTesterBlockingStub =
    UniversalTesterGrpc.blockingStub(channel)

  def shutdown(): Unit = channel.shutdown()

  def sendMessage(fieldName: String, value: Any): Unit = {
    val msgToSend = fieldName match {
      case "singleInt"    => UniversalMessage(singleInt = Some(value.asInstanceOf[Int]))
      case "singleString" => UniversalMessage(singleString = Some(value.asInstanceOf[String]))
      case "singleBool"   => UniversalMessage(singleBool = Some(value.asInstanceOf[Boolean]))
      case _              => throw new IllegalArgumentException(s"Unknown field: $fieldName")
    }
    val response: UniversalMessage = blockingStub.sendUniversal(msgToSend)
    println(s"Sent field $fieldName with value $value, got response: $response")
  }

  def sendAllMessages(message: UniversalMessage): Unit = {
    val setFields = getSetFields(message)
    setFields.foreach {
      case (fieldName, value) =>
        println(s"Sending field $fieldName with value $value")
        sendMessage(fieldName, value)
    }
  }

  def getSetFields(message: UniversalMessage): Seq[(String, Any)] =
    message.productIterator
      .zip(message.productElementNames)
      .flatMap {
        case (Some(v), name)                        => Some(name -> v) // Optional field is set
        case (seq: Seq[_], name) if seq.nonEmpty    => Some(name -> seq) // Repeated field
        case (map: Map[_, _], name) if map.nonEmpty => Some(name -> map) // Map field
        case _                                      => None // Not set
      }
      .toSeq

}

object RuntimeOptionalUpdater {

  // Update an optional Int field at runtime
  def updateOptionalInt(msg: UniversalMessage, fieldName: String, value: Any): Either[String, UniversalMessage] =
    fieldName match {
      case "singleInt" =>
        value match {
          case v: Int => Right(msg.update(_.singleInt := v))
          case _      => Left(s"Invalid type for $fieldName: ${value.getClass}")
        }
      case _ => Left(s"Unknown field: $fieldName")
    }
}
