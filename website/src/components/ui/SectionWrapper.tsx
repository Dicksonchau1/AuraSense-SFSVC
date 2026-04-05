import type { ReactNode } from 'react';

interface SectionWrapperProps {
  id?: string;
  className?: string;
  children: ReactNode;
  dark?: boolean;
}

export function SectionWrapper({
  id,
  className = '',
  children,
  dark = false,
}: SectionWrapperProps) {
  return (
    <section
      id={id}
      className={`${dark ? 'bg-surface-raised' : ''} ${className}`.trim()}
    >
      <div className="max-w-7xl mx-auto px-6 py-20 lg:py-28">{children}</div>
    </section>
  );
}
