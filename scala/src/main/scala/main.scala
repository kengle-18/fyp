// import org.slf4j.LoggerFactory

// object Main {
//   private val logger = LoggerFactory.getLogger(getClass)
//   def main(args: Array[String]): Unit = {
//     logger.info("Enter your name:")
//     val name = scala.io.StdIn.readLine()
//     logger.info(s"Hello, $name!")
//   }
// }

package com.example

import io.grpc.{ManagedChannel, ManagedChannelBuilder, Server, ServerBuilder}
import scala.concurrent.{ExecutionContext, Future}
import java.util.concurrent.Executors
import org.slf4j.LoggerFactory

// Import the generated ScalaPB classes
import com.example.base._ // adjust based on the generated folder

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
}

class Test {

  def test(): Unit =
    println("test")

  def abc(): Unit =
    println("function abc called")
}

// 4️⃣ Main Application
object Main extends App {
  private val logger = LoggerFactory.getLogger(getClass)
  private val test = new Test()

  val serverExecutionContext =
    ExecutionContext.fromExecutor(Executors.newSingleThreadExecutor())
  val server = new GrpcServer(serverExecutionContext)
  server.start()

  val client = new GrpcClient("localhost", 50051)

  try {
    // logger.info("Enter your name:")
    val name = if (args.nonEmpty) args(0) else "DockerUser"
    if (args(0).nonEmpty) {
      logger.info(s"Name from args0: ${args(0)}")
      test.test()
    }
    if (args(1).nonEmpty) {
      logger.info(s"Name from args1: ${args(1)}")
      test.abc()
    }
    logger.info(s"Hello, $name!")
    client.greet(name)
  } finally {
    logger.info(s"end")
    client.shutdown()
    server.stop()
    System.exit(0)
  }
}
