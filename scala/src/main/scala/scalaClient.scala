package com.example

import io.grpc.{ManagedChannel, ManagedChannelBuilder}
import org.slf4j.LoggerFactory
import com.example.base._ // adjust based on the generated folder

// 2️⃣ Main Client Application
object ScalaClient extends App {
  private val logger = LoggerFactory.getLogger(getClass)

  // Optional: read name from args
  val name = if (args.nonEmpty) args(0) else "Default"

  val client = new GrpcClient("scala-server", 50051)

  try client.greet(name)
  finally client.shutdown()
}
