import { Toast } from "bootstrap";
import { useEffect, useRef, useState } from "preact/hooks";

interface useToastProps {
  autohide: boolean;
  delay: number;
  onHide: () => void;
}

export default function useToast({ autohide, delay, onHide }: useToastProps) {
  const [isVisible, setIsVisible] = useState(false);

  const toastRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const el = toastRef.current;
    if (!el) {
      return;
    }
    const toastElement = new Toast(el, { autohide, delay });
    el.addEventListener("hidden.bs.toast", onHide);

    if (isVisible) {
      toastElement.show();
    } else {
      toastElement.hide();
    }

    return () => {
      el.removeEventListener("hidden.bs.toast", onHide);
      toastElement.dispose();
    };
  }, [isVisible, autohide, delay, onHide]);

  const showToast = () => {
    setIsVisible(true);
  };

  const hideToast = () => {
    setIsVisible(false);
  };

  return { toastRef, showToast, hideToast };
}
