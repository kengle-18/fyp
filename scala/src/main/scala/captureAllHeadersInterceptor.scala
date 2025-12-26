package com.example

import io.grpc._
import scala.collection.mutable

class CaptureAllHeadersInterceptor extends ServerInterceptor {

  private val timeStampKey =
    Metadata.Key.of("timestamp", Metadata.ASCII_STRING_MARSHALLER)

  override def interceptCall[ReqT, RespT](
      call: ServerCall[ReqT, RespT],
      headers: Metadata,
      next: ServerCallHandler[ReqT, RespT]
  ): ServerCall.Listener[ReqT] = {

    println("=== ALL INCOMING HEADERS ===")

    val allHeaders = mutable.Map[String, String]()
    headers.keys().forEach { key =>
      val value = headers.get(Metadata.Key.of(key, Metadata.ASCII_STRING_MARSHALLER))
      if (value != null)
        allHeaders.put(key, value)
      println(s"$key -> $value")
    }

    // Print every header (system + custom)
    // headers.keys().forEach { key =>
    //   val value =
    //     headers.get(Metadata.Key.of(key, Metadata.ASCII_STRING_MARSHALLER))
    //   println(s"$key -> $value")
    // }

    // Optionally, store timestamp in gRPC Context to access in service
    val ctx = Context
      .current()
      .withValue(
        TimestampContextKey.key,
        headers.get(timeStampKey)
      )
      .withValue(AllHeadersContextKey.key, allHeaders.toMap)

    Contexts.interceptCall(ctx, call, headers, next)
  }
}

// Context key to access timestamp in service methods
object TimestampContextKey {
  val key: Context.Key[String] = Context.key("timestamp")
}

object AllHeadersContextKey {
  val key: Context.Key[Map[String, String]] = Context.key("all-headers")
}
