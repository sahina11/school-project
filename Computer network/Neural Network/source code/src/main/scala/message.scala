package prototype

import akka.actor.ActorRef
import scala.collection.mutable.Map

case class Path(path:String) 
case class SupervisorNumber(n:Int)
case class SupervisorPing(neuronPerNode:Int)
case class WholeArray(array:Array[ActorRef])
case class NeighborMap(map:Map[ActorRef,Int])


case object Ping 
case object CreateNeuron
case object Test
case object NeighborPing
case object StartFirstFire

//Neuron Specific Messages

// Feedback for Reweighting,if value 1 increase, otherwise decrease
case object Ack
case object Nak
// Fire ignoring threshold (from stimulus)
case object DirectFire
// Fire taking threshold into consideration
case class GetFired(weight: Int)
// send to Log
case object Log
// Reconnect command
case object Reconnect
// Reset the potential
case object Reset


//new
case class NeuronMap(map:Map[ActorRef,Int]) //for observer