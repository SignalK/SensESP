import { Toast } from "bootstrap";
import { useEffect, useRef, useState } from "react";

interface useToastProps {
  autohide: boolean;
  delay: number;
  onHide: () => void;
}

export default function useToast({ autohide, delay, onHide }: useToastProps) {
  const [isVisible, setIsVisible] = useState(false);

  const toastRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (toastRef.current) {
      const toastElement = new Toast(toastRef.current, { autohide, delay });
      // Call onHide when the toast is hidden to sync the state
      toastRef.current.addEventListener("hidden.bs.toast", onHide);

      if (isVisible) {
        toastElement.show();
      } else {
        toastElement.hide();
      }
    }
  }, [isVisible]);

  const showToast = () => {
    setIsVisible(true);
  };

  const hideToast = () => {
    setIsVisible(false);
  };

  return { toastRef, showToast, hideToast };
}
