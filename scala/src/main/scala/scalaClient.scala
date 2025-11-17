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

  // val msg1 = msg0.update(
  //   _.optionalSingleInt := Some(42),
  //   _.optionalSingleBool := Some(true)
  // )
  val msg1 = UniversalMessage().update(
    // ---------- Scalar fields ----------
    _.singleInt := 42,
    _.bigInt := 123456789L,
    _.singleString := "hello",
    _.singleBool := true,
    _.singleDouble := 3.14,
    _.singleFloat := 2.718f,
    _.singleBytes := com.google.protobuf.ByteString.copyFromUtf8("bytes"),
    // ---------- Repeated fields ----------
    _.repeatedInt :+= 1,
    _.repeatedInt :+= 2,
    _.repeatedString :+= "foo",
    _.repeatedBool :+= true,
    _.repeatedDouble :+= 1.1,
    _.repeatedFloat :+= 2.2f,
    _.repeatedBigInt :+= 1000000000L,
    _.repeatedBytes :+= com.google.protobuf.ByteString.copyFromUtf8("bar"),
    // ---------- Map fields ----------
    _.mapIntString(1) := "one",
    _.mapStringInt("a") := 100,
    _.mapIntNested(1) := UniversalMessage.NestedMessage(name = Some("nested1"), value = Some(10)),
    // ---------- Nested ----------
    _.nested := UniversalMessage.NestedMessage(name = Some("top"), value = Some(99)), // Remove Some()
    _.repeatedNested :+= UniversalMessage.NestedMessage(name = Some("r1")),
    _.repeatedNested :+= UniversalMessage.NestedMessage(name = Some("r2"), value = Some(2)),
    // ---------- Enum ----------
    _.status := UniversalMessage.Status.ACTIVE,
    _.repeatedStatus :+= UniversalMessage.Status.INACTIVE
  )

  try client.sendAllMessages(msg1)
  finally client.shutdown()
}
