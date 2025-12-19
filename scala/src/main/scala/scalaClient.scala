package com.example

import io.grpc.{ManagedChannel, ManagedChannelBuilder}
import org.slf4j.LoggerFactory
import com.example.base._ // adjust based on the generated folder
import com.example.grpcHelper._

// 2️⃣ Main Client Application
object ScalaClient extends App {
  private val logger = LoggerFactory.getLogger(getClass)

  // Get host/port from env or fall back to defaults
  val host = sys.env.getOrElse("SERVER_HOST", "localhost")
  val port = sys.env.getOrElse("SERVER_PORT", "50051").toInt

  val client = new GrpcClient(host, port)

  var msgUpdated: UniversalMessage = UniversalMessage()

  Config.init(args)
  val allArgs: Vector[Vector[String]] = Config.getAllArgsInLine()
  for ((args, line) <- allArgs.zipWithIndex) {
    msgUpdated = client.setFieldsWithConfigValues(args, msgUpdated)
    println(msgUpdated)
  }

  try client.sendAllMessages(msgUpdated)
  finally client.shutdown()
}
