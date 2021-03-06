#!/bin/sh


FILE="$HOME/.afterstep/.postcard"

# Explanation.


# bail if file doesn't exist
if [ -r $FILE ]
then
   echo -n
else
   exit
fi

# bail if mail isn't available
if which mail 1>/dev/null 2>/dev/null;
then 
   echo -n
else
   exit
fi

explanation ()
{
echo "
Please send the developers an postcard!

And finally we are coming to the fun part. AS it is the only compensation that
developers of this lousy software get - AfterStep will automagically e-mail a
single message anytime ~/.afterstep directory needs to be rebuilt. This message
will contain such a vital information as date, time, your X server color depth,
number of X screens, number of xinerama screens, compiler version, kernel
version, and list of linked to libraries.

If you object to this e-mailing activity, simply deny it below.

That is all! We will go back to your regular programming now.

Oh, almost forgot - enjoy your eyecandy :)
"
}

# View
view ()
{
      ${PAGER:-"cat"} $FILE
      prompt
}

prompt ()
{
   # this block is for BSD vs SYS V compatibility
   ##########
   if [ "`echo -n`" = "-n" ]
   then
      n=""
      c="\c"
   else
      n="-n"
      c=""
   fi
   ##########

   echo Please send our developers a postcard:
   echo $n [Allow], Deny, View, Later? $c
   read ans
   

   case $ans in
   [aA]*)
      allow;;
   [dD]*)
      deny;;
   [vV]*)
      view;;
   [lLqQ]*)
      later;;
   *)
      allow;;
   esac
}

allow()
{
   send_postcard
   exit;
}

deny()
{
   rename_postcard ".denied"
   echo "aborting..."
   exit
}

later()
{
   echo "maybe later ..."
   remove_postcard
   exit
}

rename_postcard()
{
   mv $FILE $FILE$1
   echo "To resend or reattempt sending this postcard, see $FILE$1"
}

send_postcard()
{
   echo -n "sending postcard ... "

   mail -s "AfterStep installation info" sasha@aftercode.net < $FILE &&
   { echo " Sent."; rename_postcard ".sent"; } ||
   { echo " Send failed."; rename_postcard ".failed"; }
}

end()
{
   echo
   echo "To quit, press return."
   read ans
}

explanation
prompt
end
