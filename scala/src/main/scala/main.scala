package com.example

import io.grpc.{ManagedChannel, ManagedChannelBuilder, Server, ServerBuilder}
import scala.concurrent.{ExecutionContext, Future}
import java.util.concurrent.Executors
import org.slf4j.LoggerFactory

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

// 2️⃣ gRPC Server
class GrpcServer(executionContext: ExecutionContext) {

  val server: Server = ServerBuilder
    .forPort(50051)
    .addService(GreeterGrpc.bindService(new GreeterImpl, executionContext))
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

  def shutdown(): Unit = channel.shutdown()

  def greet(name: String): Unit = {
    val request = HelloRequest(name)
    val response = blockingStub.sayHello(request)
    println(s"Client: Greeting received: ${response.message}")
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
    // logger.info("Enter your name:")
    // val name = if (args.nonEmpty) args(0) else "Default"
    // if (args.length > 0 && args(0).nonEmpty)
    //   logger.info(s"Name from args0: ${args(0)}")
    // if (args.length > 1 && args(1).nonEmpty)
    //   logger.info(s"Name from args1: ${args(1)}")

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

    msg1.productIterator.zip(msg1.productElementNames).foreach {
      case (value, name) =>
        // println(value, name)
        value match {
          case Some(v)                        => println(s"$name is set: $v")
          case seq: Seq[_] if seq.nonEmpty    => println(s"$name is set: $seq")
          case map: Map[_, _] if map.nonEmpty => println(s"$name is set: $map")
          case _                              => // field not set
        }
    }

    // Test for runtime
    import RuntimeOptionalUpdater._
    val msg2 = updateOptionalInt(msg, "singleInt", 42)
    println(s"Updated: ${msg2.map(_.singleInt)}")

  } finally {
    client.shutdown()
    server.stop()
    System.exit(0)
  }
}
