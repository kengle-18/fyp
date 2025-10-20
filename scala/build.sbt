import Dependencies._
import sbt.plugins.JvmPlugin
import sbtassembly.AssemblyPlugin
import sbtassembly.AssemblyPlugin.autoImport._
import scalapb.compiler.Version.scalapbVersion
import sbtassembly.MergeStrategy

ThisBuild / scalaVersion := "2.13.12"
ThisBuild / version := "0.1.0-SNAPSHOT"
ThisBuild / organization := "com.example"
ThisBuild / organizationName := "example"

val scalapbVersion = "0.11.11"
val grpcJavaVersion = "1.57.0"

lazy val root = (project in file("."))
  .enablePlugins(JvmPlugin, AssemblyPlugin)
  .settings(
    name := "scala",
    libraryDependencies ++= Seq(
      "org.slf4j" % "slf4j-api" % "1.7.32",
      "ch.qos.logback" % "logback-classic" % "1.2.11",
      "com.thesamet.scalapb" %% "scalapb-runtime" % scalapbVersion,
      "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapb.compiler.Version.scalapbVersion,
      // "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
      "io.grpc" % "grpc-netty-shaded" % grpcJavaVersion,
      "io.grpc" % "grpc-protobuf" % scalapb.compiler.Version.grpcJavaVersion,
      "io.grpc" % "grpc-stub" % scalapb.compiler.Version.grpcJavaVersion,
      "com.chuusai" %% "shapeless" % "2.3.10"
    ),
    // Required to tell ScalaPB to generate the gRPC service code
    // Read proto files in src/main/protobuf/*.proto
    Compile / PB.targets := Seq(
      scalapb.gen(grpc = true) -> (Compile / sourceManaged).value
    ),
    // This compiler plugin is needed for the ScalaPB code generation
    addCompilerPlugin(
      "com.thesamet.scalapb" %% "compilerplugin" % scalapbVersion
    ),
    libraryDependencies += munit % Test,
    assembly / assemblyJarName := s"${name.value}-assembly-${version.value}.jar",
    assembly / assemblyMergeStrategy := {
      case PathList("META-INF", "io.netty.versions.properties") =>
        MergeStrategy.first
      case PathList("META-INF", "services", xs @ _*) =>
        MergeStrategy.concat // ✅ keep ServiceLoader entries
      case PathList("META-INF", xs @ _*) =>
        MergeStrategy.discard // still discard other META-INF junk
      case PathList("module-info.class") =>
        MergeStrategy.discard
      case x =>
        val oldStrategy = (assembly / assemblyMergeStrategy).value
        oldStrategy(x)
    }
  )
