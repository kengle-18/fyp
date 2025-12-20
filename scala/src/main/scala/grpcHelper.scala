package com.example.grpcHelper

import io.grpc.{ManagedChannel, ManagedChannelBuilder, Server, ServerBuilder}
import scala.concurrent.{ExecutionContext, Future}
import java.util.concurrent.Executors
import org.slf4j.LoggerFactory
import java.io.{BufferedWriter, FileWriter}
import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable.ArrayBuffer
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

// Import the generated ScalaPB classes
import com.example.base._ // adjust based on the generated folder

sealed trait Loggable
case class FullMessage(message: UniversalMessage) extends Loggable
case class SingleField(fieldName: String, value: Option[Any]) extends Loggable
case class SimpleString(message: String) extends Loggable

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
    // println(s"Sent field $fieldName with value $value, got response: $responseField")
    (fieldName, responseField)
  }

  def sendAllMessages(message: UniversalMessage): Unit = {
    val setFields = getSetFields(message)

    val timestamp: String = LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyyMMdd_HHmmss_SSS")) + "_"

    val dir: String = sys.env.getOrElse("CONFIG_APP_TEXT_DIR", "/app/scala/src/generated")
    val dirPath: Path = Paths.get(dir)

    // val filePath: String = sys.env.getOrElse("CONFIG_APP_TEXT_FILE", "scalaClient.txt")
    val scalaMessageInital: String =
      timestamp + sys.env.getOrElse("OUTPUT_FILE_SCALA_MESSAGE_INITAL", "scalaMessageInital.txt")
    val scalaMessageIndiviualField: String =
      timestamp + sys.env.getOrElse("OUTPUT_FILE_SCALA_MESSAGE_INDIVIDUAL_FIELD", "scalaMessageIndiviualField.txt")
    val scalaResponseFromServer: String =
      timestamp + sys.env.getOrElse("OUTPUT_FILE_SCALA_RESPONSE_FROM_SERVER", "scalaResponseFromServer.txt")

    // val fullFilePath: String = dirPath.resolve(filePath).toString
    val scalaMessageInitalPath: String = dirPath.resolve(scalaMessageInital).toString
    val scalaMessageIndiviualFieldPath: String = dirPath.resolve(scalaMessageIndiviualField).toString
    val scalaResponseFromServerPath: String = dirPath.resolve(scalaResponseFromServer).toString

    // The message to send to server
    writeToFile(
      headerMessage = s"=== Sending full message to server ===",
      endingMessage = s"=== End of full message ===",
      data = FullMessage(message),
      dirPath = dirPath,
      fullFilePath = scalaMessageInitalPath,
      appendMode = false
    )

    // Send all set fields in the message
    val reponseAll: UniversalMessage = blockingStub.sendUniversal(message)
    writeToFile(
      headerMessage = s"=== Response from server ===",
      endingMessage = s"=== End of full message ===",
      data = FullMessage(reponseAll),
      dirPath = dirPath,
      fullFilePath = scalaResponseFromServerPath,
      appendMode = false
    )

    // Send each set field individually
    writeToFile(
      data = SimpleString("=== Response for seding individual fields to server ==="),
      dirPath = dirPath,
      fullFilePath = scalaMessageIndiviualFieldPath,
      appendMode = true
    )
    setFields.foreach {
      case (fieldName, value) =>
        // println(s"Sending field $fieldName with value $value")
        val responseIndividualFields: (String, Option[Any]) = sendMessage(fieldName, value)

        /** Same as
        val (fieldName, valueOpt) = responseIndividualFields
        val data = SingleField(fieldName, valueOpt)*/

        val data = (SingleField.apply _).tupled(responseIndividualFields)
        writeToFile(
          data = data,
          dirPath = dirPath,
          fullFilePath = scalaMessageIndiviualFieldPath,
          appendMode = true
        )
    }
    writeToFile(
      data = SimpleString("=== End of individual fields ==="),
      dirPath = dirPath,
      fullFilePath = scalaMessageIndiviualFieldPath,
      appendMode = true
    )

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
      data: Loggable,
      dirPath: Path,
      fullFilePath: String,
      appendMode: Boolean,
      headerMessage: String = "",
      endingMessage: String = ""
  ): Unit = {
    if (!Files.exists(dirPath))
      Files.createDirectories(dirPath)
    val writer = new BufferedWriter(new FileWriter(fullFilePath, appendMode))
    try {
      if (headerMessage.nonEmpty)
        writer.write(headerMessage + "\n")

      data match {
        case FullMessage(msg) =>
          writer.write("Message fields:\n")
          msg.productIterator.zip(msg.productElementNames).foreach {
            case (Some(v), name)        => writer.write(s"$name: $v\n")
            case (seq: Seq[_], name)    => writer.write(s"$name: [${seq.mkString(", ")}]\n")
            case (map: Map[_, _], name) => writer.write(s"$name: ${map.mkString("{", ", ", "}")}\n")
            case (None, name)           => writer.write(s"$name: <not set>\n")
            case (_, name)              => writer.write(s"$name: \n")
          }

        case SingleField(name, valueOpt) =>
          valueOpt match {
            case Some(v: Seq[_])    => writer.write(s"$name: [${v.mkString(", ")}]\n")
            case Some(v: Map[_, _]) => writer.write(s"$name: ${v.mkString("{", ", ", "}")}\n")
            case Some(v)            => writer.write(s"$name: $v\n")
            case None               => writer.write(s"$name: <not set>\n")
          }

        case SimpleString(msg) =>
          writer.write(msg + "\n")
      }
    } finally {
      if (endingMessage.nonEmpty)
        writer.write(endingMessage + "\n\n")
      writer.close()
    }
  }

  def setFieldsWithConfigValues(configValues: Vector[String], message: UniversalMessage): UniversalMessage = {
    var updatedMessage: UniversalMessage = message
    for (Seq(flag, value) <- configValues.grouped(2)) {
      // print(s"flag:$flag, value:$value\n")

      val (prefix, resultFlag) = extractPrefixAndresultFlag(flag)
      // print(s"prefix: $prefix, flag: $resultFlag\n")

      prefix match {
        case 's' =>
          updatedMessage = helperSetAllIndividualFields(message = updatedMessage, flag = resultFlag, value = value)
        case 'r' =>
          updatedMessage = helperSetAllRepeatedFields(message = updatedMessage, flag = resultFlag, value = value)
        case 'm' =>
          updatedMessage = helperSetAllMapFields(message = updatedMessage, flag = resultFlag, value = value)
        case 'n' =>
          updatedMessage = helperSetNestedFields(message = updatedMessage, flag = resultFlag, value = value)
        case 'x' =>
          updatedMessage = helperSetStatusFields(message = updatedMessage, flag = resultFlag, value = value)
        case _ => println("Default")
      }
    }
    // println(s"message : $updatedMessage")
    updatedMessage
  }

  def helperSetAllIndividualFields(message: UniversalMessage, flag: String, value: String): UniversalMessage = {
    // println(s"Matching flag: '$flag' with value: '$value'")
    val result = flag match {
      case "i"   => message.update(_.optionalSingleInt := Some(value.toInt))
      case "bi"  => message.update(_.optionalBigInt := Some(value.toLong))
      case "s"   => message.update(_.optionalSingleString := Some(value))
      case "b"   => message.update(_.optionalSingleBool := Some(value.toBoolean))
      case "d"   => message.update(_.optionalSingleDouble := Some(value.toDouble))
      case "f"   => message.update(_.optionalSingleFloat := Some(value.toFloat))
      case "bts" => message.update(_.optionalSingleBytes := Some(com.google.protobuf.ByteString.copyFromUtf8(value)))
      case _     => message
    }
    // println(s"Result after update: $result")
    result
  }

  def helperSetAllRepeatedFields(message: UniversalMessage, flag: String, value: String): UniversalMessage = {
    // println(s"Matching flag: '$flag' with value: '$value'")
    val result = flag match {
      case "i"   => message.update(_.repeatedInt :+= value.toInt)
      case "bi"  => message.update(_.repeatedBigInt :+= value.toLong)
      case "s"   => message.update(_.repeatedString :+= value)
      case "b"   => message.update(_.repeatedBool :+= value.toBoolean)
      case "d"   => message.update(_.repeatedDouble :+= value.toDouble)
      case "f"   => message.update(_.repeatedFloat :+= value.toFloat)
      case "bts" => message.update(_.repeatedBytes :+= com.google.protobuf.ByteString.copyFromUtf8(value))
      case "nsi" =>
        val keyVal = helperGetMapKeyAndValueFromString(value)
        message.update(
          _.repeatedNested :+= UniversalMessage.NestedMessage(name = Option(keyVal._1), value = keyVal._2.toIntOption)
        )
      case "x" =>
        UniversalMessage.Status.fromName(value.toUpperCase) match {
          case Some(statusEnum) =>
            message.update(_.repeatedStatus :+= statusEnum) // set the enum
          case None =>
            println(s"Unknown enum value, skipping, not setting, $value")
            message // leave message unchanged
        }
      case _ => message
    }
    // println(s"Result after update: $result")
    result
  }

  def helperSetAllMapFields(message: UniversalMessage, flag: String, value: String): UniversalMessage = {
    // println(s"PlexMatching flag: '$flag' with value: '$value'")

    val keyVal = helperGetMapKeyAndValueFromString(value)
    val result = flag match {
      case "is" => message.update(_.mapIntString := message.mapIntString + (keyVal._1.toInt -> keyVal._2))
      case "si" =>
        if (keyVal._2.isEmpty)
          //default is no input
          message.update(_.mapStringInt := message.mapStringInt + (keyVal._1 -> 0))
        else message.update(_.mapStringInt := message.mapStringInt + (keyVal._1 -> keyVal._2.toInt))
      case "in" =>
        // To get all the values after = so inner key and value
        val parts = value.split(",")
        val outerKey = parts.lift(0).getOrElse("").toInt
        val innerKey = parts.lift(1).flatMap(_.split("=").lift(1)).getOrElse("")
        val innerValue = parts.lift(2).flatMap(_.split("=").lift(1)).getOrElse("")

        // println(s"first=$outerKey, innerKey=$innerKey, innerValue=$innerValue")

        val nested = UniversalMessage.NestedMessage(name = Option(innerKey), value = innerValue.toIntOption)
        // Set with the outer key and nested
        message.update(_.mapIntNested := message.mapIntNested + (outerKey -> nested))
      case _ => message
    }
    // println(s"Result after update: $result")
    result
  }

  def helperSetNestedFields(message: UniversalMessage, flag: String, value: String): UniversalMessage = {
    // println(s"Matching flag: '$flag' with value: '$value'")

    val keyVal = helperGetMapKeyAndValueFromString(value)
    val result = flag match {
      case "si" =>
        message.update(
          _.nested := UniversalMessage.NestedMessage(name = Option(keyVal._1), value = keyVal._2.toIntOption)
        )
      case _ => message
    }
    // println(s"Result after update: $result")
    result
  }

  def helperSetStatusFields(message: UniversalMessage, flag: String, value: String): UniversalMessage = {
    // println(s"Matching flag: '$flag' with value: '$value'")

    val statusEnumOption: Option[UniversalMessage.Status] =
      UniversalMessage.Status.fromName(value.toUpperCase)
    val result = statusEnumOption match {
      case Some(statusEnum) =>
        message.update(_.status := statusEnum) // set the enum
      case None =>
        println(s"Unknown enum value, skipping, not setting, $value")
        message // leave message unchanged
    }
    // println(s"Result after update: $result")
    result
  }

  def extractPrefixAndresultFlag(input: String): (Char, String) = {
    // Get the prefix
    val cleaned = if (input.startsWith("--")) input.drop(2) else input
    val prefix = cleaned(0)
    // get rest of flag in flag
    val resultFlag = cleaned.substring(1)
    // println(s"cleaned: $cleaned, prefix: $prefix, resultFlag: $resultFlag")
    (prefix, resultFlag)
  }

  def helperGetMapKeyAndValueFromString(value: String): (String, String) = {
    val firstComma = value.indexOf(',')

    // No comma found
    if (firstComma == -1) return ("", "")

    val key = value.substring(0, firstComma)
    val v = value.substring(firstComma + 1)

    (key, v)
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
