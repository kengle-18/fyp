package com.example.grpcHelper

import io.grpc.{ManagedChannel, ManagedChannelBuilder, Server, ServerBuilder}
import scala.concurrent.{ExecutionContext, Future}
import java.util.concurrent.Executors
import org.slf4j.LoggerFactory
import java.io.{BufferedWriter, FileWriter}
import java.nio.file.{Files, Path, Paths}

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

  def sendMessage(fieldName: String, value: Any): (String, Option[Any]) = {
    val msgToSend = fieldName match {
      // Scalar fields
      case "singleInt"    => UniversalMessage(singleInt = Some(value.asInstanceOf[Int]))
      case "bigInt"       => UniversalMessage(bigInt = Some(value.asInstanceOf[Long]))
      case "singleString" => UniversalMessage(singleString = Some(value.asInstanceOf[String]))
      case "singleBool"   => UniversalMessage(singleBool = Some(value.asInstanceOf[Boolean]))
      case "singleDouble" => UniversalMessage(singleDouble = Some(value.asInstanceOf[Double]))
      case "singleFloat"  => UniversalMessage(singleFloat = Some(value.asInstanceOf[Float]))
      case "singleBytes"  => UniversalMessage(singleBytes = Some(value.asInstanceOf[com.google.protobuf.ByteString]))

      // Repeated fields
      case "repeatedInt"    => UniversalMessage(repeatedInt = value.asInstanceOf[Seq[Int]])
      case "repeatedString" => UniversalMessage(repeatedString = value.asInstanceOf[Seq[String]])
      case "repeatedBool"   => UniversalMessage(repeatedBool = value.asInstanceOf[Seq[Boolean]])
      case "repeatedDouble" => UniversalMessage(repeatedDouble = value.asInstanceOf[Seq[Double]])
      case "repeatedFloat"  => UniversalMessage(repeatedFloat = value.asInstanceOf[Seq[Float]])
      case "repeatedBigInt" => UniversalMessage(repeatedBigInt = value.asInstanceOf[Seq[Long]])
      case "repeatedBytes"  => UniversalMessage(repeatedBytes = value.asInstanceOf[Seq[com.google.protobuf.ByteString]])

      // Map fields
      case "mapIntString" => UniversalMessage(mapIntString = value.asInstanceOf[Map[Int, String]])
      case "mapStringInt" => UniversalMessage(mapStringInt = value.asInstanceOf[Map[String, Int]])
      case "mapIntNested" =>
        UniversalMessage(mapIntNested = value.asInstanceOf[Map[Int, UniversalMessage.NestedMessage]])

      // Nested message
      case "nested" => UniversalMessage(nested = Some(value.asInstanceOf[UniversalMessage.NestedMessage]))
      case "repeatedNested" =>
        UniversalMessage(repeatedNested = value.asInstanceOf[Seq[UniversalMessage.NestedMessage]])

      // Enum
      case "status"         => UniversalMessage(status = Some(value.asInstanceOf[UniversalMessage.Status]))
      case "repeatedStatus" => UniversalMessage(repeatedStatus = value.asInstanceOf[Seq[UniversalMessage.Status]])

      case _ => throw new IllegalArgumentException(s"Unknown field: $fieldName")
    }
    val response: UniversalMessage = blockingStub.sendUniversal(msgToSend)

    val responseField = response.productIterator
      .zip(response.productElementNames)
      .find { case (_, name) => name == fieldName }
      .flatMap {
        case (Some(v), _)                        => Some(v) // Optional field is set
        case (seq: Seq[_], _) if seq.nonEmpty    => Some(seq) // Repeated field
        case (map: Map[_, _], _) if map.nonEmpty => Some(map) // Map field
        case _                                   => None // Not set
      }
    println(s"Sent field $fieldName with value $value, got response: $responseField")
    (fieldName, responseField)
  }

  def sendAllMessages(message: UniversalMessage): Unit = {
    val setFields = getSetFields(message)

    val dir: String = sys.env.getOrElse("CONFIG_APP_TEXT_DIR", "/app/scala/src/generated")
    val dirPath: Path = Paths.get(dir)
    val filePath: String = sys.env.getOrElse("CONFIG_APP_TEXT_FILE", "scalaClient.txt")
    val fullFilePath: String = dirPath.resolve(filePath).toString

    // The message to send to server
    writeToFile(
      headerMessage = s"=== Sending full message to server ===",
      universalmessageToSend = message,
      dirPath = dirPath,
      fullFilePath = fullFilePath,
      appendMode = false
    )

    // Send all set fields in the message
    val reponseAll: UniversalMessage = blockingStub.sendUniversal(message)
    writeToFile(
      headerMessage = s"=== Response from server ===",
      universalmessageToSend = reponseAll,
      dirPath = dirPath,
      fullFilePath = fullFilePath,
      appendMode = true
    )

    // Send each set field individually
    setFields.foreach {
      case (fieldName, value) =>
        // println(s"Sending field $fieldName with value $value")
        val responseIndividualFields = sendMessage(fieldName, value)
        writeToFileField(
          field = responseIndividualFields,
          dirPath = dirPath,
          fullFilePath = fullFilePath,
          appendMode = true
        )
    }

    println("Sending completed.")
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

  def writeToFile(
      universalmessageToSend: UniversalMessage,
      dirPath: Path,
      fullFilePath: String,
      appendMode: Boolean,
      headerMessage: String = "=== Empty Header===\n"
  ): Unit = {
    if (!Files.exists(dirPath))
      Files.createDirectories(dirPath)

    val writer = new BufferedWriter(new FileWriter(fullFilePath, appendMode)) // append mode true, false to overwrite
    try {
      if (headerMessage.nonEmpty)
        writer.write(s"$headerMessage\n")

      writer.write("Message fields:\n")

      universalmessageToSend.productIterator
        .zip(universalmessageToSend.productElementNames)
        .foreach {
          case (Some(value), name) =>
            writer.write(s"$name: $value\n")
          case (seq: Seq[_], name) =>
            writer.write(s"$name: [${seq.mkString(", ")}]\n")
          case (map: Map[_, _], name) =>
            writer.write(s"$name: ${map.mkString("{", ", ", "}")}\n")
          case (None, name) =>
            writer.write(s"$name: <not set>\n")
          case (_, name) =>
            writer.write(s"$name: \n")
        }
    } finally {
      writer.write("=== End of Message ===\n\n")
      writer.close()
    }

  }

  // Individual field writer
  def writeToFileField(
      field: (String, Option[Any]),
      dirPath: Path,
      fullFilePath: String,
      appendMode: Boolean,
      headerMessage: String = "=== Empty Header===\n"
  ): Unit = {
    val (fieldName, valueOpt) = field
    if (!Files.exists(dirPath))
      Files.createDirectories(dirPath)

    val writer = new BufferedWriter(new FileWriter(fullFilePath, appendMode))
    try {
      if (headerMessage.nonEmpty) writer.write(headerMessage + "\n")

      valueOpt match {
        case Some(v: Seq[_])    => writer.write(s"$fieldName: [${v.mkString(", ")}]\n")
        case Some(v: Map[_, _]) => writer.write(s"$fieldName: ${v.mkString("{", ", ", "}")}\n")
        case Some(v)            => writer.write(s"$fieldName: $v\n")
        case None               => writer.write(s"$fieldName: <not set>\n")
      }
    } finally {
      writer.write("=== End of Message ===\n\n")
      writer.close()
    }
  }

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
