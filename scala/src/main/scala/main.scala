import org.slf4j.LoggerFactory

object Main {
  private val logger = LoggerFactory.getLogger(getClass)
  
  def main(args: Array[String]): Unit = {
    println("Enter your name:")
    val name = scala.io.StdIn.readLine()
    logger.info(s"Hello, $name!")
  }
}
