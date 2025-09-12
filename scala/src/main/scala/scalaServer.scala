package com.example

import io.grpc.{Server, ServerBuilder}
import scala.concurrent.{ExecutionContext, Future}
import java.util.concurrent.Executors
import org.slf4j.LoggerFactory

// Import the generated ScalaPB classes
import com.example.base._ // adjust based on the generated folder

// 3️⃣ Main Server Application
object ScalaServer extends App {
  private val logger = LoggerFactory.getLogger(getClass)

  val serverExecutionContext =
    ExecutionContext.fromExecutor(Executors.newSingleThreadExecutor())
  val server = new GrpcServer(serverExecutionContext)

  sys.addShutdownHook {
    logger.info("Shutting down gRPC server...")
    server.stop()
    logger.info("Server stopped.")
  }

  server.start()
  logger.info("Server is running. Press Ctrl+C to stop.")
  server.blockUntilShutdown()
}
