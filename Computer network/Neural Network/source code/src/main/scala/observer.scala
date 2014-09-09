package prototype

import akka.actor._
import akka.util._
import scala.concurrent.duration._
import java.util.Date
import scala.collection.mutable.Set
import scala.collection.mutable.Map

class Observer extends Actor {

  val timeout = 3 seconds
  val inputRatio = 0.05

  var oldSet = Set[Int]()
  var nowSet = Set[Int]()

  var wholeMap = Map[ActorRef,Int]()

  var input = scala.collection.immutable.Set[ActorRef]()
  var firstFireToggle = false

  //set receive timeout
  context.setReceiveTimeout( timeout )

  def receive = {
    case m:NeuronMap =>
      wholeMap = m.map

      input = wholeMap.slice(0, (wholeMap.size*inputRatio).toInt).keys.toSet
    case StartFirstFire =>
      println("First fire\n")
      firstFireToggle = true

      input.foreach( i => i ! DirectFire)
    case Log =>
      nowSet += wholeMap(sender)
    case ReceiveTimeout => 
      if (oldSet.size == 0) {
        if (firstFireToggle) {
          println("observer: Number of neurons firing == " + nowSet.size)
          println("observer: who is firing: \n" + nowSet.mkString("-"))
          println()
        }
        oldSet = nowSet
        nowSet.clear
      }
      else {
        if (firstFireToggle) {
          println("observer: Number of neurons firing == " + nowSet.size)
          println("observer: who is firing:\n" + nowSet.mkString("-"))
          println()
        }
        oldSet = nowSet
        nowSet.clear
      }
      
      if (firstFireToggle) input.foreach( i => i ! DirectFire)


  }

}

