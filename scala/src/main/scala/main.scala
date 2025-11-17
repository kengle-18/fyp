package com.example

import io.grpc.{ManagedChannel, ManagedChannelBuilder, Server, ServerBuilder}
import scala.concurrent.{ExecutionContext, Future}
import java.util.concurrent.Executors
import org.slf4j.LoggerFactory

import java.nio.file.{Files, Paths}
import java.nio.charset.StandardCharsets

// Import the generated ScalaPB classes
import com.example.base._ // adjust based on the generated folder

import shapeless._
import shapeless.labelled._
import shapeless.syntax.singleton._

// 1️⃣ Server Implementation
class GreeterImpl extends GreeterGrpc.Greeter {

  override def sayHello(request: HelloRequest): Future[HelloReply] =
    Future.successful(HelloReply(message = s"Hello, ${request.name}!"))
}

class UniversalTesterImpl extends UniversalTesterGrpc.UniversalTester {

  override def sendUniversal(request: UniversalMessage): Future[UniversalMessage] =
    Future.successful(request) // simply echo back for testing
}

// 2️⃣ gRPC Server
class GrpcServer(executionContext: ExecutionContext) {

  val server: Server = ServerBuilder
    .forPort(50051)
    .addService(GreeterGrpc.bindService(new GreeterImpl, executionContext))
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

  val blockingStub: GreeterGrpc.GreeterBlockingStub =
    GreeterGrpc.blockingStub(channel)

  val blockingStubV2: UniversalTesterGrpc.UniversalTesterBlockingStub =
    UniversalTesterGrpc.blockingStub(channel)

  def shutdown(): Unit = channel.shutdown()

  def greet(name: String): Unit = {
    val request = HelloRequest(name)
    val response = blockingStub.sayHello(request)
    println(s"Client: Greeting received: ${response.message}")
  }

  def sendMessage(fieldName: String, value: Any): Unit = {
    val msgToSend = fieldName match {
      case "singleInt"    => UniversalMessage(singleInt = Some(value.asInstanceOf[Int]))
      case "singleString" => UniversalMessage(singleString = Some(value.asInstanceOf[String]))
      case "singleBool"   => UniversalMessage(singleBool = Some(value.asInstanceOf[Boolean]))
      case _              => throw new IllegalArgumentException(s"Unknown field: $fieldName")
    }
    val response: UniversalMessage = blockingStubV2.sendUniversal(msgToSend)
    println(s"Sent field $fieldName with value $value, got response: $response")
  }

  // update using generics
  def updateField[T, Repr <: HList, K <: Symbol, V](
      t: T,
      field: Witness.Aux[K],
      value: Option[V]
  )(implicit
      gen: LabelledGeneric.Aux[T, Repr],
      updater: ops.record.Updater.Aux[Repr, FieldType[K, Option[V]], Repr]
  ): T = {
    val repr = gen.to(t)
    val fieldEntry = (field.value ->> value).asInstanceOf[FieldType[K, Option[V]]]
    val updated = updater(repr, fieldEntry)
    gen.from(updated)
  }

  def getSetFields(msg: UniversalMessage): Seq[(String, Any)] =
    msg.productIterator
      .zip(msg.productElementNames)
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

// 4️⃣ Main Application
object Main extends App {
  private val logger = LoggerFactory.getLogger(getClass)

  val serverExecutionContext =
    ExecutionContext.fromExecutor(Executors.newSingleThreadExecutor())
  val server = new GrpcServer(serverExecutionContext)

  Config.init(args)

  val name = Config.getArg(0)
  val option = Config.getArg(1)
  logger.info(s"Name selected: $name")
  logger.info(s"Option selected: $option")

  val allArgs = Config.getAll(Seq("Default", "Default"))
  logger.info(s"All args merged: ${allArgs.mkString(", ")}")

  server.start()

  val client = new GrpcClient("localhost", 50051)

  try {
    case class User(name: Option[String] = None, age: Option[Int] = None)
    val user = User(Some("Bob"), Some(30))

    client.greet(name)

    val updatedUser1 = client.updateField(user, Witness('name), Some("Alice"))
    println(s"Updated name: $updatedUser1")

    val updatedUser2 = client.updateField(user, Witness('age), Some(35))
    println(s"Updated age: $updatedUser2")

    val updatedUser3 = client.updateField(updatedUser2, Witness('name), Some("Charlie"))
    println(s"Updated both: $updatedUser3")

    // Test for generics compile time
    val msg = UniversalMessage()
    val msg1 = msg.update(
      _.optionalSingleInt := Some(42),
      _.optionalSingleBool := Some(true)
    )
    println(s"msg is : ${msg1}")
    println(s"msg: ${msg1.singleInt}")

    val setFields = client.getSetFields(msg1)

    setFields.foreach {
      case (fieldName, value) =>
        println(s"Sending field $fieldName with value $value")
        client.sendMessage(fieldName, value)
    }

    Files.write(
      Paths.get("/app/scala/src/generated/example.txt"),
      "Hello Scala!".getBytes(StandardCharsets.UTF_8)
    )

    // Test for runtime
    import RuntimeOptionalUpdater._
    val msg2 = updateOptionalInt(msg, "singleInt", 42)
    println(s"Updated: ${msg2.map(_.singleInt)}")

    client.greet(name)

  } finally {
    client.shutdown()
    server.stop()
    System.exit(0)
  }
}
