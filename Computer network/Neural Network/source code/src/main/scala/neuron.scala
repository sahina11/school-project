package prototype

import akka.actor._
import scala.util.Random
import scala.concurrent.duration._

class Neuron extends Actor {
  val threshold = 10 // predefined
  var potential = 0
  val timeout = 2 seconds


  var neighborMap = scala.collection.mutable.Map[ActorRef, Int]()
  var firedBy = scala.collection.mutable.Set[ActorRef]()
  var scheduler = context.system.scheduler.schedule(timeout,timeout,self, Reset)(context.system.dispatcher,self)
  scheduler.cancel

  var serverPath = ""
  var observerPath =""

  def receive = {
    case m:Path =>
      serverPath = m.path
      observerPath = m.path + "/observer"

      context.actorSelection(serverPath) ! Ping
    case m:NeighborMap =>
      neighborMap = m.map

      context.actorSelection(serverPath) ! NeighborPing  
    case DirectFire =>
      potential = 0
      neighborMap.keys.foreach( i => i ! GetFired(neighborMap(i)))
      scheduler = context.system.scheduler.schedule(timeout,timeout,self, Reset)(context.system.dispatcher,self) 
  
      context.actorSelection(observerPath) ! Log
    case m:GetFired =>
      potential += m.weight
      firedBy += sender
      context.actorSelection(observerPath) ! Log

      if (potential >= threshold) {
        neighborMap.keys.foreach( i => i ! GetFired(neighborMap(i)))
        firedBy.foreach( i => i ! Ack)
        potential = 0
        firedBy.clear

        scheduler.cancel
        scheduler = context.system.scheduler.schedule(timeout,timeout,self, Reset)(context.system.dispatcher,self) 
      }
    case Ack =>
      neighborMap(sender) += 1
    case Reset =>
      firedBy.foreach(i => i ! Nak)
      potential = 0
      firedBy.clear
    case Nak =>
      if (neighborMap(sender) == 1) neighborMap(sender) =0
      else neighborMap(sender) -= 1  
    case Test =>
      println("\tTest message!! ( " + self + " )")

  }
}

