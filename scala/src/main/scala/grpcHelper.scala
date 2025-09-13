package com.example.grpcHelper

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
class GrpcServer(executionContext: ExecutionContext, port: Int) {

  val server: Server = ServerBuilder
    .forPort(port)
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
