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

  Config.init(args)
  val name = Config.getArg(0)
  val option = Config.getArg(1)
  logger.info(s"Name selected: $name")
  logger.info(s"Option selected: $option")

  val allArgs = Config.getAll(Seq("Default", "Default"))
  logger.info(s"All args merged: ${allArgs.mkString(", ")}")

  // Test for scala self grpc using lens
  val msg0 = UniversalMessage()

  val msg1 = msg0.update(
    _.optionalSingleInt := Some(42),
    _.optionalSingleBool := Some(true)
  )

  try client.sendAllMessages(msg1)
  finally client.shutdown()
}
