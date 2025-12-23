package com.example

import io.grpc._

class CaptureAllHeadersInterceptor extends ServerInterceptor {

  private val timeStampKey =
    Metadata.Key.of("timestamp", Metadata.ASCII_STRING_MARSHALLER)

  override def interceptCall[ReqT, RespT](
      call: ServerCall[ReqT, RespT],
      headers: Metadata,
      next: ServerCallHandler[ReqT, RespT]
  ): ServerCall.Listener[ReqT] = {

    println("=== ALL INCOMING HEADERS ===")

    // Print every header (system + custom)
    headers.keys().forEach { key =>
      val value =
        headers.get(Metadata.Key.of(key, Metadata.ASCII_STRING_MARSHALLER))
      println(s"$key -> $value")
    }

    // Optionally, store timestamp in gRPC Context to access in service
    val ctx = Context
      .current()
      .withValue(
        TimestampContextKey.key,
        headers.get(timeStampKey)
      )

    Contexts.interceptCall(ctx, call, headers, next)
  }
}

// Context key to access timestamp in service methods
object TimestampContextKey {
  val key: Context.Key[String] = Context.key("timestamp")
}
