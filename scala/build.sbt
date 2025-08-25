// import Dependencies._

// ThisBuild / scalaVersion     := "2.13.12"
// ThisBuild / version          := "0.1.0-SNAPSHOT"
// ThisBuild / organization     := "com.example"
// ThisBuild / organizationName := "example"

// lazy val root = (project in file("."))
//   .settings(
//     name := "scala",
//     libraryDependencies += munit % Test
//   )

// // See https://www.scala-sbt.org/1.x/docs/Using-Sonatype.html for instructions on how to publish to Sonatype.

import Dependencies._
import sbt.plugins.JvmPlugin
import sbtassembly.AssemblyPlugin
import sbtassembly.AssemblyPlugin.autoImport._


ThisBuild / scalaVersion := "2.13.12"
ThisBuild / version := "0.1.0-SNAPSHOT"
ThisBuild / organization := "com.example"
ThisBuild / organizationName := "example"

lazy val root = (project in file("."))
  .enablePlugins(JvmPlugin, AssemblyPlugin)
  .settings(
    name := "scala",
    libraryDependencies += munit % Test,
    assembly / assemblyJarName := s"${name.value}-assembly-${version.value}.jar"
  )