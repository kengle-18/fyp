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

// lazy val root = (project in file("."))
//   .enablePlugins(JvmPlugin, AssemblyPlugin)
//   .settings(
//     name := "scala",
//     libraryDependencies ++= Seq(
//       "org.slf4j" % "slf4j-api" % "1.7.32",
//       "ch.qos.logback" % "logback-classic" % "1.2.11",
//       "com.thesamet.scalapb" %% "scalapb-runtime" % scalapbVersion % "protobuf",
//       "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapb.compiler.Version.scalapbVersion,
//       "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
//       "io.grpc" % "grpc-protobuf" % scalapb.compiler.Version.grpcJavaVersion,
//       "io.grpc" % "grpc-stub" % scalapb.compiler.Version.grpcJavaVersion,
//     ),
//     libraryDependencies += munit % Test,
//     assembly / assemblyJarName := s"${name.value}-assembly-${version.value}.jar",
//     assembly / assemblyMergeStrategy := {
//     case PathList("META-INF", "io.netty.versions.properties") =>
//       MergeStrategy.first
//     case PathList("META-INF", xs @ _*) =>
//       MergeStrategy.discard
//     case PathList("module-info.class") =>
//       MergeStrategy.discard
//     case x =>
//       val oldStrategy = (assembly / assemblyMergeStrategy).value
//       oldStrategy(x)
//     },
//     Compile / PB.targets := Seq(
//       scalapb.gen(grpc = true) -> (Compile / sourceManaged).value
//     )
//   )

// lazy val root = (project in file("."))
//   .enablePlugins(JvmPlugin, AssemblyPlugin)
//   .settings(
//     scalaVersion := "2.13.12",
//     libraryDependencies ++= Seq(
//       "org.slf4j" % "slf4j-api" % "1.7.32",
//       "ch.qos.logback" % "logback-classic" % "1.2.11",
//       "com.thesamet.scalapb" %% "scalapb-runtime" % scalapbVersion,
//       "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapbVersion,
//       "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
//       "io.grpc" % "grpc-protobuf" % scalapb.compiler.Version.grpcJavaVersion,
//       "io.grpc" % "grpc-stub" % scalapb.compiler.Version.grpcJavaVersion
//     ),
//     Compile / PB.targets := Seq(
//       scalapb.gen(grpc = true) -> (Compile / sourceManaged).value
//     ),
//     addCompilerPlugin("com.thesamet.scalapb" %% "compilerplugin" % scalapbVersion),
//     libraryDependencies += munit % Test,
//     assembly / assemblyJarName := s"${name.value}-assembly-${version.value}.jar"
//   )

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
      "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
      "io.grpc" % "grpc-protobuf" % scalapb.compiler.Version.grpcJavaVersion,
      "io.grpc" % "grpc-stub" % scalapb.compiler.Version.grpcJavaVersion,
    ),
    // Required to tell ScalaPB to generate the gRPC service code
    Compile / PB.targets := Seq(
      scalapb.gen(grpc = true) -> (Compile / sourceManaged).value
    ),
    // This compiler plugin is needed for the ScalaPB code generation
    addCompilerPlugin("com.thesamet.scalapb" %% "compilerplugin" % scalapbVersion),

    libraryDependencies += munit % Test,
    assembly / assemblyJarName := s"${name.value}-assembly-${version.value}.jar",
    // These merge strategies are necessary to avoid conflicts when creating a single fat JAR
    assembly / assemblyMergeStrategy := {
      case PathList("META-INF", "io.netty.versions.properties") =>
        MergeStrategy.first
      case PathList("META-INF", xs @ _*) =>
        MergeStrategy.discard
      case PathList("module-info.class") =>
        MergeStrategy.discard
      case x =>
        val oldStrategy = (assembly / assemblyMergeStrategy).value
        oldStrategy(x)
    }
  )

